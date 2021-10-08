//********************************************************************************//
//  Sopranino T-Stick 2GW - LOLIN D32 PRO - USB -WiFi                             //
//  Input Devices and Music Interaction Laboratory (IDMIL)                        //
//  Created:  February 2018 by Alex Nieva                                         //
//            October 2019 by Edu Meneses - firmware version 19101 (2019-10.v1)   //
//  Notes   : Based on test program for reading CY8C201xx using I2C               //
//            by Joseph Malloch 2011                                              //
//                                                                                //
//            Adapted to work with Arduino IDE 1.8.10 and T-Stick Sopranino 2GW   //
//********************************************************************************//

//**************************************************//
// WiFi32Manager - For use with ESP8266 or ESP32    //
//                                                  //
// Created originally for the T-Stick project:      //
// http://www-new.idmil.org/project/the-t-stick/    //
//                                                  //
// This code uses code (fork) of 3 other projects:  //
// https://github.com/kentaylor/WiFiManager         //
// https://github.com/tzapu/WiFiManager             //
// https://github.com/zhouhan0126/WIFIMANAGER-ESP32 //
//                                                  //
// Edu Meneses - IDMIL - Mar 2019                   //
//**************************************************//

//**************************************************//
// MIMU - Magnetometer + accelerometer + gyroscope  //
// orientation library                              //
//                                                  //
// https://github.com/DocSunset/MIMU                //
//                                                  //
//                                                  //
// Travis West - IDMIL - Oct 2019                   //
//**************************************************//


// READ ALL DEPENDENCIES AND OBSERVATIONS BEFORE UPLOAD !!!!!!!!
// ---- --- ------------ --- ------------ ------ ------ 
// ---- --- ------------ --- ------------ ------ ------ 


// IMPORTANT: You need to upload a file (data/config.json) into ESP32 filesystem. 
// Follow the instructions at https://github.com/me-no-dev/arduino-esp32fs-plugin

// DEPENDENCIES:
// esp32 board library (add url to preferences; install via board manager)
// esp8266 board library (add url to preferences; install via board manager)
// Wifi32Manager https://github.com/edumeneses/WiFi32Manager
// Adafruit_LSM9DS1 (library manager)
// ArduinoJSON - https://github.com/bblanchon/ArduinoJson
// OSC - https://github.com/CNMAT/OSC (v3.5.7)
// MIMU - https://github.com/DocSunset/MIMU
// Eigen - https://github.com/bolderflight/Eigen

//  OBS:
//  1-) Use esp32 1.0.4 or newer (https://github.com/espressif/arduino-esp32/releases).
//  2-) Also install ESP8266 board library even if you'll use the ESP32 (https://github.com/esp8266/Arduino)
//  3-) MINU library complains if you keep any IMU-related files other than MIMU_LSM9DS1.h and MIMU_LSM9DS1.cpp
//  4-) Board currently in use: LOLIN D32 PRO.


#include <FS.h>

#define ESP32; // define ESP8266 or ESP32 according to your microcontroller.
//#define ESP8266;

#if defined(ESP32)
#include "SPIFFS.h"
#endif

#include <ArduinoJson.h>  // https://github.com/bblanchon/ArduinoJson

#include <WiFi32Manager.h> // https://github.com/edumeneses/WiFi32Manager
// already includes:
// Wifi.h (https://github.com/esp8266/Arduino) or ESP8266WiFi.h (https://github.com/esp8266/Arduino)
// AND
// WebServer.h or ESP8266WebServer.h
// AND
// DNSServer.h

// https://github.com/CNMAT/OSC (v3.5.7)
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>

#include <Wire.h>
#include <SPI.h>
#include <MIMU_LSM9DS1.h> // https://github.com/DocSunset/MIMU
                          // requires SparkFunLSM9DS1 library - https://github.com/sparkfun/SparkFun_LSM9DS1_Arduino_Library
#include <MIMUCalibrator.h>
#include <MIMUFusion.h>


//////////////////////////////////
//////////////////////////////////
// T-Stick Specific Definitions //
//////////////////////////////////
//////////////////////////////////

struct Tstick {
  int id;
  char type[4];
  char author[20];
  char color[20];
  char APpasswd[15];
  char lastConnectedNetwork[30];
  char lastStoredPsk[30];
  int32_t firmware;
  char oscIP[17];
  int32_t oscPORT;
  float FSRoffset;
  byte touchMask[2];
  float abias[3];
  float mbias[3];
  float gbias[3];
  float acclcalibration[9];
  float magncalibration[9];
  float gyrocalibration[9];
};

Tstick Tstick;

struct DataStruct {
  byte touch[2];
  float fsr;
  float piezo;
  float accl[3];
  float gyro[3];
  float magn[3];
  float raw[9];
  float quat[4];
  float ypr[3];
};

DataStruct Data = {{0,0},0,0,{0,0,0},{0,0,0},{0,0,0},{0,0,0,0,0,0,0,0,0},{0,0,0,0},{0,0,0}};

IPAddress osc_IP; // used to send OSC messages
char APpasswdTemp[15]; // used to check before save new T-Stick passwd
char APpasswdValidate[15]; // used to check before save new T-Stick passwd
char tstickSSID[30] = "T-Stick_";
char tstickID[5];

//////////////////////
// WiFi Definitions //
//////////////////////

WiFiUDP oscEndpoint; // A UDP instance to let us send and receive packets over UDP
const unsigned int portLocal = 8888; // local port to listen for OSC packets (not used for sending)

//////////////
// defaults //
//////////////

int piezoPin = 32;
int fsrPin = 33;
const int buttonPin = 15;
int buttonState = 0; // variable for reading the pushbutton status
int waitForConnection = 8000; // set waiting time for connecting to a WiFi network
byte touchInterval = 15; // interval between capsense readings
unsigned long touchLastRead = 0; // track capsense last sensor read
int serialInterval = 1000; // interval between serial prints for sensor data
unsigned long serialLastRead = 0; // track capsense last sensor read
unsigned long time_now = millis(); // variable used for LED, serial print, and WiFi connection 
                                   // (updated by blinkLED() every loop)
char wifimanagerbuf[12];

///////////////
// blink LED //
///////////////

int ledPin = 5;
bool ledStatus = false;
int long_blink = 1000;
int short_blink = 500;
int flickering = 150;

void blinkLED(int ledInterval) {
  if (millis() > time_now + ledInterval) {
    time_now = millis();
    ledStatus = !ledStatus;
    digitalWrite(ledPin, ledStatus);
    } 
}

///////////////////////
// MIMU Library Init //
///////////////////////

MIMU_LSM9DS1 mimu{}; // use default SDA and SCL as per board library macros
MIMUCalibrator calibrator{};
MIMUFusionFilter filter{};


///////////
///////////
// setup //
///////////
///////////

void setup() {

  Serial.begin(115200);

  Serial.println("\n");
  Serial.println("*******************************************************************************");
  Serial.println("*  Sopranino T-Stick 2GW - LOLIN D32 PRO - USB -WiFi                          *");
  Serial.println("*  Input Devices and Music Interaction Laboratory (IDMIL)                     *");
  Serial.println("*  Created: February 2018 by Alex Nieva                                       *");
  Serial.println("*           October 2019 by Edu Meneses - firmware version 19101 (2019-10.v1) *");
  Serial.println("*  Notes:   Based on test program for reading CY8C201xx using I2C             *");
  Serial.println("*           by Joseph Malloch 2011                                            *");
  Serial.println("                                                                              *");
  Serial.println("*  Adapted to work with Arduino IDE 1.8.10 and T-Stick Sopranino 2GW          *");
  Serial.println("*******************************************************************************");
  Serial.println("\n");

  Serial.println("Setting up T-Stick...\n");

  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  ledStatus = true;
  digitalWrite(ledPin, ledStatus);

  // Start FS and check Json file (config.json)
  mountFS();

  // Print Json file stored values
  printJSON();
  
  // Load Json file stored values
  parseJSON();

  // Print T-Stick Specific Definitions
  printVariables();

  // Connect to saved WiFi
  connectToWifi();

  // Starting IMU
  initIMU();

  // Starting Capsense
  initCapsense();

  Serial.println("\nT-Stick setup complete.\n");
  
}


//////////
//////////
// loop //
//////////
//////////

void loop() {

  // Calling WiFiManager configuration portal on demand:
  buttonState = digitalRead(buttonPin);
  if ( buttonState == LOW ) {
    digitalWrite(ledPin, 0);
    Wifimanager_portal(tstickSSID, Tstick.APpasswd);
    }

  // reading sensor data
  readData();

  // send data (OSC)
  sendOSC();

  // receiving OSC
  receiveOSC();

  // printing sensor data (serial port)
  //printData();

  // LED modes:
  // ON = Setup mode
  // long blink = not connected to network
  // short blink = connected and sending data
  // flickering =  receiving OSC
  if ( WiFi.status() == WL_CONNECTED ) {blinkLED(flickering);}
  else {blinkLED(short_blink);}
}
