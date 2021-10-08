//********************************************************************************//
//  Sopranino T-Stick 2GW - LOLIN D32 PRO - USB -WiFi                             //
//  Input Devices and Music Interaction Laboratory (IDMIL)                        //
//  Created:  February 2018 by Alex Nieva                                         //
//            March 2020 by Edu Meneses - firmware version 200330 (2020/Mar/30)   //
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

// Tested on Wemos Lolin D32 Pro

// DEPENDENCIES:
// esp32 board library (add url to preferences; install via board manager)
// esp8266 board library (add url to preferences; install via board manager)
// Wifi32Manager https://github.com/edumeneses/WiFi32Manager
// Adafruit_LSM9DS1 (library manager)
// ArduinoJSON - https://github.com/bblanchon/ArduinoJson (v6.14.1)
// OSC - https://github.com/CNMAT/OSC (v3.5.7)
// MIMU - https://github.com/DocSunset/MIMU
// Eigen - https://github.com/bolderflight/Eigen

//  OBS:
//  1-) Use esp32 1.0.4 or newer (https://github.com/espressif/arduino-esp32/releases).
//  2-) Also install ESP8266 board library even if you'll use the ESP32 (https://github.com/esp8266/Arduino)
//  3-) MINU library complains if you keep any IMU-related files other than MIMU_LSM9DS1.h and MIMU_LSM9DS1.cpp
//  4-) Board currently in use: LOLIN D32 PRO.


#include <FS.h>

#define ESP32 // define ESP8266 or ESP32 according to your microcontroller.
//#define TSTICK193; // define if flashing the T-Stick #193.

#if defined(ESP32)
#include "platforms/ESP32.h"
#elif defined(ESP8266)
#include "platforms/ESP8266.h"
#else
#error Missing platform.
#endif

#include <ArduinoJson.h>  // https://github.com/bblanchon/ArduinoJson

#include <WiFi32Manager.h> // https://github.com/edumeneses/WiFi32Manager
// already includes:
// Wifi.h or ESP8266WiFi.h (https://github.com/esp8266/Arduino)
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

#include<stdlib.h> //floats to string


//////////////////////////////////
//////////////////////////////////
// T-Stick Specific Definitions //
//////////////////////////////////
//////////////////////////////////

const int32_t firmware = 200330;

struct Tstick {
  int id;
  char type[4];
  char author[20];
  char color[20];
  char APpasswd[15];
  char lastConnectedNetwork[30];
  char lastStoredPsk[30];
  int32_t firmware;
  byte osc;
  char oscIP[2][17];
  int32_t oscPORT[2];
  byte libmapper;
  int16_t FSRoffset;
  byte touchMask[8];
  float abias[3];
  float mbias[3];
  float gbias[3];
  float acclcalibration[9];
  float magncalibration[9];
  float gyrocalibration[9];
} Tstick;

struct RawDataStruct {
  byte touch[8]; // /raw/capsense, i..., 0--255, ... (1 int per 8 capacitive stripes -- 8 bits)
  byte touchStrips[64];
  int fsr; // /raw/fsr, i, 0--4095
  int piezo; // /raw/piezo, i, 0--1023
  float accl[3]; // /raw/accl, iii, +/-32767 (integers)
  float gyro[3]; // /raw/gyro, fff, +/-34.90659 (floats)
  float magn[3]; // /raw/magn, fff, +/-32767 (integers)
  float raw[10]; // /raw (IMU data to be send to callibration app)
  float quat[4]; // /raw/quat, ffff, ?, ? ,? ,?
  float magAccl;
  float magGyro;
  float magMagn;
  byte buttonShort; // /raw/button/short, i, 0 or 1
  byte buttonLong; // /raw/button/long, i, 0 or 1
  byte buttonDouble; // /raw/button/double, i, 0 or 1
} RawData;

struct NormDataStruct {
  float fsr; // /norm/fsr, f, 0--1
  float piezo; // /norm/piezo, f, 0--1
  float accl[3]; // /norm/accl, fff, +/-1, +/-1, +/-1
  float gyro[3]; // /norm/gyro, fff, +/-1, +/-1, +/-1
  float magn[3]; // /norm/magn, fff, +/-1, +/-1, +/-1
} NormData;

struct LastStateDataStruct {
  int blobPos[4];
  int gyroArrayCounter;
  float gyroXArray[5];
  float gyroYArray[5];
  float gyroZArray[5];
} LastState;

struct InstrumentDataStruct {
  float touchAll; // /instrument/touch/all, f, 0--1
  float touchTop; // /instrument/touch/top, f, 0--1
  float touchMiddle; // /instrument/touch/middle, f, 0--1
  float touchBottom; // /instrument/touch/bottom, f, 0--1
  float brush; // /instrument/touch/brush, f, 0--? (~cm/s)
  float multiBrush[4]; // /instrument/touch/brush/multibrush, ffff, 0--? (~cm/s)
  float rub; // /instrument/touch/rub, f, 0--? (~cm/s)
  float multiRub[4]; // /instrument/touch/rub/multirub, ffff, 0--? (~cm/s)
  float ypr[3]; // /instrument/ypr, fff, +/-180, +/-90 ,+/-180 (degrees)
  float shakeXYZ[3]; // /instrument/shakexyz, fff, 0--?
  float jabXYZ[3]; // /instrument/jabxyz, fff, 0--?
} InstrumentData;

struct Capsense { 
  byte answer1, answer2;
} capsense;

struct blob { 
  byte blobArray[8]; // shows the "center" of each array
  int blobPos[4];  // position (index) of each blob
  float blobSize[4]; // "size" of each blob
} BlobDetection;


// IPAddress oscIP; // used to send OSC messages
char APpasswdTemp[15]; // used to check before save new T-Stick passwd
char APpasswdValidate[15]; // used to check before save new T-Stick passwd
char tstickSSID[30] = "T-Stick_";
char tstickID[5];
float battery;
byte batteryPercentage;
int batteryInterval = 1000;
unsigned long  batteryLastRead = 0;
unsigned long  batteryLastSend = 0;
byte batteryCount = 0;

///////////////////////////////////
// Variables for Instrument Data //
///////////////////////////////////

byte touchSizeEdge = 4; // amount of T-Stick stripes for top and bottom portions of the T-Stick (arbitrary)


//////////////////////
// WiFi Definitions //
//////////////////////

WiFiUDP oscEndpoint; // A UDP instance to let us send and receive packets over UDP
const unsigned int portLocal = 8888; // local port to listen for OSC packets (not used for sending)

//////////////
// defaults //
//////////////

byte piezoPin = 33;
byte fsrPin = 32;
byte batteryPin = 35;
const int buttonPin = 15;
byte buttonState = 0; // variable for reading the pushbutton status
const int buttonShortInterval = 300;
const int buttonLongInterval = 2000;
unsigned long buttonTimer;
byte buttonShortFlag = 0;
byte buttonLongFlag = 0;
unsigned long buttonDoubleTimer;
byte buttonDoubleFlag = 0;
int waitForConnection = 8000; // set waiting time for connecting to a WiFi network
int serialInterval = 1000; // interval between serial prints for sensor data
unsigned long serialLastRead = 0; // track capsense last sensor read
unsigned long time_now = millis(); // variable used for LED, serial print, battery reading, and WiFi connection 
                                   // (updated by blinkLED() every loop)
float fsrbuf;

#ifdef TSTICK193
      const int sda1Pin = 21;
      const int scl1Pin = 22;
      const int sda2Pin  = 5;
      const int scl2Pin = 4;
      const int pwm1Pin = 14;
      const int pwm2Pin = 15;
#endif

////////////////////////////////
// Leaky integrator variables //
////////////////////////////////

const int leakyBrushFreq = 100; // leaking frequency (Hz)
unsigned long leakyBrushTimer = 0;
const int leakyRubFreq = 100;
unsigned long leakyRubTimer = 0;
byte brushCounter[4];
const int leakyShakeFreq = 10;
unsigned long leakyShakeTimer[3];

///////////////
// blink LED //
///////////////

#ifdef TSTICK193
      const int ledPin = 12;
  #else
      const int ledPin = 5;
#endif

bool ledStatus = false;
int long_blink = 1000;
int short_blink = 600;
int flickering = 80;

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
  Serial.println("*********************************************************************************");
  Serial.println("*  Sopranino T-Stick 2GW - LOLIN D32 PRO - USB -WiFi                            *");
  Serial.println("*  Input Devices and Music Interaction Laboratory (IDMIL)                       *");
  Serial.println("*  Created: February 2018 by Alex Nieva                                         *");
  Serial.println("*           February 2020 by Edu Meneses - firmware version 200207 (2020-02-20) *");
  Serial.println("*  Notes:   Based on test program for reading CY8C201xx using I2C               *");
  Serial.println("*           by Joseph Malloch 2011                                              *");
  Serial.println("                                                                                *");
  Serial.println("*  Adapted to work with Arduino IDE 1.8.10 and T-Stick Sopranino 2GW            *");
  Serial.println("*********************************************************************************");
  Serial.println("\n");

  Serial.println("Setting up T-Stick...\n");

  Tstick.firmware = firmware;

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
  capsense_scan(); // Look for Capsense boards and return their addresses
                   // must run before initLibmapper to get # of capsense boards

  // Starting libmapper
  if (Tstick.libmapper == 1) {
    initLibmapper();
  }
  
  Serial.println("\nT-Stick setup complete.\n");
  
}


//////////
//////////
// loop //
//////////
//////////

void loop() {

  // Calling WiFiManager configuration portal on demand:
  if ( RawData.buttonLong == 1 ) {
    #ifdef TSTICK193
      digitalWrite(ledPin, 1);
    #else
      digitalWrite(ledPin, 0);
    #endif
    RawData.buttonLong = 0; buttonLongFlag = 0;
    Wifimanager_portal(tstickSSID, Tstick.APpasswd);
    }

  // reading sensor data
  readData();

  // updating high-level gestural descriptors
  updateInstrument();

  // send data (OSC)
  if (Tstick.osc == 1) {
    sendOSC(Tstick.oscIP[0], Tstick.oscPORT[0]);
    if (strcmp(Tstick.oscIP[1],"0.0.0.0") != 0 ) {
        if (strcmp(Tstick.oscIP[1],Tstick.oscIP[0]) == 0) {
          if (Tstick.oscPORT[0] != Tstick.oscPORT[1]) {
            sendOSC(Tstick.oscIP[1], Tstick.oscPORT[1]);
          }
        } else {
          sendOSC(Tstick.oscIP[1], Tstick.oscPORT[1]);
        }
    }
  }

  // Update libmapper
  if (Tstick.libmapper == 1) {
    updateLibmapper();
  }
  
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
  else {blinkLED(long_blink);}
}
