//********************************************************************************//
//  Sopranino T-Stick 2GW - LOLIN D32 PRO - USB -WiFi                             //
//  Input Devices and Music Interaction Laboratory (IDMIL)                        //
//  Created:  February 2018 by Alex Nieva                                         //
//            October 2019 by Edu Meneses - firmware version 200214 (2020/Feb/20) //
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
// ArduinoJSON - https://github.com/bblanchon/ArduinoJson (v6.14.1)
// OSC - https://github.com/CNMAT/OSC (v3.5.7)
// MIMU - https://github.com/DocSunset/MIMU
// Eigen - https://github.com/bolderflight/Eigen

//  OBS:
//  1-) Use esp32 1.0.4 or newer (https://github.com/espressif/arduino-esp32/releases).
//  2-) Also install ESP8266 board library even if you'll use the ESP32 (https://github.com/esp8266/Arduino)
//  3-) MINU library complains if you keep any IMU-related files other than MIMU_LSM9DS1.h and MIMU_LSM9DS1.cpp
//  4-) Board currently in use: LOLIN D32 PRO.

#include "TStick.h"

struct Tstick Tstick;
RawDataStruct RawData;
NormDataStruct NormData;
LastStateDataStruct LastStateData;
InstrumentDataStruct InstrumentData;
Capsense capsense;

//////////////////////////////////
//////////////////////////////////
// T-Stick Specific Definitions //
//////////////////////////////////
//////////////////////////////////

const int32_t firmware = 200220;

IPAddress osc_IP; // used to send OSC messages
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

//////////////////////
// WiFi Definitions //
//////////////////////

WiFiUDP oscEndpoint; // A UDP instance to let us send and receive packets over UDP
const unsigned int portLocal = 8888; // local port to listen for OSC packets (not used for sending)

//////////////
// defaults //
//////////////

byte piezoPin = 32;
byte fsrPin = 33;
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
  // printData();

  // LED modes:
  // ON = Setup mode
  // long blink = not connected to network
  // short blink = connected and sending data
  // flickering =  receiving OSC
  if ( WiFi.status() == WL_CONNECTED ) {blinkLED(flickering);}
  else {blinkLED(long_blink);}
}
