//********************************************************************************//
//  Sopranino T-Stick 2GW - LOLIN D32 PRO / TinyPico - USB - WiFi                 //
//  Input Devices and Music Interaction Laboratory (IDMIL)                        //
//  Created:  February 2018 by Alex Nieva                                         //
//            March 2021 by Edu Meneses - firmware version 2011105 (2021/Nov/05)  //
//                                                                                //
//            Adapted to work with Arduino IDE 1.8.15 and T-Stick Sopranino 2GW   //
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

// Tested on Wemos Lolin D32 Pro and the TinyPico

// DEPENDENCIES:
// esp32 board library (add url to preferences; install via board manager)
// esp8266 board library (add url to preferences; install via board manager)
// Wifi32Manager https://github.com/edumeneses/WiFi32Manager
// Adafruit_LSM9DS1 (library manager)
// ArduinoJSON - https://github.com/bblanchon/ArduinoJson (v6.14.1)
// OSC - https://github.com/CNMAT/OSC (v3.5.7)
// MIMU - https://github.com/DocSunset/MIMU
//     For MIMU:
//       Eigen - (library manager)
//       SparkFun LSM9DS1 - (library manager)
// Libmapper-arduino (https://github.com/mathiasbredholt/libmapper-arduino)
// Trill (Bela) - https://github.com/BelaPlatform/Trill-Arduino
// TinyPico helper Library - (library manager)

//  OBS:
//  1-) Use esp32 1.0.4 or newer (https://github.com/espressif/arduino-esp32/releases).
//  2-) Also install ESP8266 board library even if you'll use the ESP32 (https://github.com/esp8266/Arduino)
//  3-) MINU library complains if you keep any IMU-related files other than MIMU_LSM9DS1.h and MIMU_LSM9DS1.cpp
//  4-) Boards currently in use: TinyPico and LOLIN D32 PRO
//  5-) There's an error with the ESP32 inet.h file. The file requires manual fixing. Check the issue at 
//      https://github.com/mathiasbredholt/libmapper-arduino/issues/3 

// define IDMIL_ESP8266, IDMIL_LOLIN, or IDMIL_TINYPICO according to your microcontroller.
//#define IDMIL_ESP8266
//#define IDMIL_LOLIN 
#define IDMIL_TINYPICO

//#define TSTICK193; // define if flashing the T-Stick #193.

#define TRILL // define this to disable the IDMIL's capsense and use the Bela Trill

#define LIBMAPPER // define this to enable libmapper code


#include <FS.h>

#if defined(IDMIL_LOLIN)
  #include "SPIFFS.h"
  #include "esp_wifi.h"
#elif defined(IDMIL_TINYPICO)
  #include "SPIFFS.h"
  #include "esp_wifi.h"
  #include <TinyPICO.h>
  TinyPICO tp = TinyPICO();
#elif defined(IDMILESP8266)
  // nothing to include
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

#ifdef TRILL
  #include <Trill.h>
  Trill trillSensor; // for Trill Craft
  Trill trillSensor2; // for Trill Craft
  byte touchStripsSize = 30; // number of stripes
  byte trillamount = 1;
#endif

#include<stdlib.h> //floats to string


//////////////////////////////////
//////////////////////////////////
// T-Stick Specific Definitions //
//////////////////////////////////
//////////////////////////////////

const int32_t firmware = 211105;

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

char sn[3] = "SN";
char sp[3] = "SP";
char al[3] = "AL";
char tn[3] = "TN";

struct RawDataStruct {
  byte touch[8]; // /raw/capsense, i..., 0--255, ... (1 int per 8 capacitive stripes -- 8 bits)
  int touch_trill[30];
  int touch_trill2[30];
  byte touchStrips[60];
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

unsigned long ledtimer = 0;
unsigned int ledinterval = 1000;
unsigned int ledn = 0;
uint8_t ledcolor = 0;

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
  Serial.println("*  Sopranino T-Stick 2GW - LOLIN D32 PRO / TinyPico - USB - WiFi                *");
  Serial.println("*  Input Devices and Music Interaction Laboratory (IDMIL)                       *");
  Serial.println("*  Created:  February 2018 by Alex Nieva                                        *");
  Serial.println("*            March 2021 by Edu Meneses - firmware version 2011105 (2021/Nov/05) *");
  Serial.println("*                                                                               *");
  Serial.println("*            Adapted to work with Arduino IDE 1.8.15 and T-Stick Sopranino 2GW  *");
  Serial.println("*********************************************************************************");
  Serial.println("\n");

  Serial.println("Setting up T-Stick...\n");

  Tstick.firmware = firmware;

  #ifndef IDMIL_TINYPICO
    pinMode(ledPin, OUTPUT);
  #endif
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
  #ifndef TRILL
    capsense_scan(); // Look for Capsense boards and return their addresses
                     // must run before initLibmapper to get # of capsense boards
  #else
    trillSensor.setup(Trill::TRILL_CRAFT, 48);
    if (strcmp(Tstick.type,al) == 0 || strcmp(Tstick.type,tn) == 0) {
      trillSensor2.setup(Trill::TRILL_CRAFT, 49);
      trillamount = 2;
    }
  #endif
  
  // Starting libmapper
  #ifdef LIBMAPPER
    if (Tstick.libmapper == 1) {
      initLibmapper();
    }
  #endif

  // Setting Deep sleep wake button
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_15,0); // 1 = High, 0 = Low
  
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

  // go to deep sleep if double press button
  if (RawData.buttonDouble == 1){
      RawData.buttonDouble = 0,
      esp_deep_sleep_start();
  }

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
  #ifdef LIBMAPPER
    if (Tstick.libmapper == 1) {
      updateLibmapper();
    }
  #endif
  
  // receiving OSC
  receiveOSC();

  // printing sensor data (serial port)
  //printData();

  #ifndef IDMIL_TINYPICO
    // LED modes:
    // ON = Setup mode
    // long blink = not connected to network
    // short blink = connected and sending data
    // flickering =  receiving OSC
    if ( WiFi.status() == WL_CONNECTED ) {blinkLED(flickering);}
    else {blinkLED(long_blink);}
  #endif

  #ifdef IDMIL_TINYPICO
    if (batteryPercentage < 10) { // low battery (red)
      ledinterval = 20;
      ledcolor = ledblink(255, 20);
      tp.DotStar_SetPixelColor(ledcolor, 0, 0);
    } else {
          if (WiFi.status() == WL_CONNECTED) { // blinks when connected, cycle when disconnected
            ledinterval = 1000;
            ledcolor = ledblink(255,20);
            tp.DotStar_SetPixelColor(0, uint8_t(ledcolor/2), ledcolor);
          } else {
            ledinterval = 4000;
            ledcolor = ledcycle(ledcolor, 0, 255);
            tp.DotStar_SetPixelColor(0, uint8_t(ledcolor/2), ledcolor);
          }
      }
  #endif
  
}


// LED functions

int ledblink(int intensity, int onTime) { // onTime between 10 and 100 (percent)
  int result = 0;
  if (onTime <= 10) {
    onTime = 10;
  }
  if (onTime > 100) {
    onTime = 100;
  }
  int switchPoint = 100/onTime;
  if ((millis() - ledtimer) > ledinterval) {
    ledtimer = millis();
  }
  if ( (millis() - ledtimer) < ledinterval/switchPoint ) {
    result = intensity;
  } else {
    result = 0;
  }
  return result;
}

int ledrampUp(int currentValue, int startValue, int endValue) { // plug currentValue in startValue for a 'log' curve
  int result = currentValue;
  if (endValue > currentValue) {
    int rampStep = ledinterval/(endValue - startValue);
    if ((millis() - ledtimer) > rampStep) {
      ledtimer = millis();
      result += 1;
    }
  }
  return result;
}

int ledrampDown(int currentValue, int startValue, int endValue) { // plug currentValue in startValue for a 'log' curve
  int result = currentValue;
  if (endValue < currentValue) {
    int rampStep = ledinterval/(startValue - endValue);
    if ((millis() - ledtimer) > rampStep) {
      ledtimer = millis();
      result -= 1;
    }
  }
  return result;
}

int ledcycle(int currentValue, int minValue, int maxValue) {
  int result = currentValue;
  if (ledn <= 0) {
    ledn = 1;
  }
  if (millis()-ledtimer > ledinterval) {
    ledtimer = millis();
    ledn = 1;
  }
  int nMax = 2 * (maxValue - minValue);               // # of steps
  int rampStep = ledinterval / nMax;                // time between steps
  if (millis()-ledtimer > ledinterval/2) {        // ramping down
    if (minValue < currentValue) {
      if ((millis() - ledtimer) > rampStep*ledn) {
        ledn++;
        result -= 1;
      }
    }
  } else {                                            // ramping up
    if (maxValue > currentValue) { 
      if ((millis() - ledtimer) > rampStep*ledn) {
        ledn++;
        result += 1;
      }
    }
  }
  return result;
}
