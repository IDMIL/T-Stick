//********************************************************************************//
//  Sopranino T-Stick 2GW - LOLIN D32 PRO - USB -WiFi                             //
//  Input Devices and Music Interaction Laboratory                                //
//  Created:  February 2018 by Alex Nieva                                         //
//            March 2019 by Edu Meneses - version 19031 (2019-03.v1)              //
//  Notes   : Based on test program for reading CY8C201xx using I2C               //
//            by Joseph Malloch 2011                                              //
//                                                                                //
//            Adapted to work with Arduino IDE 1.8.8 and T-Stick Sopranino 2GW    //
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


// READ ALL DEPENDENCIES AND OBSERVATIONS BEFORE UPLOAD !!!!!!!!
// ---- --- ------------ --- ------------ ------ ------ 
// ---- --- ------------ --- ------------ ------ ------ 

// FIRMWARE VERSION: 19051

// IMPORTANT: You need to upload a blank file (data/config.json) into ESP32 filesystem. 
// Follow the instructions at https://github.com/me-no-dev/arduino-esp32fs-plugin

// DEPENDENCIES:
// esp32 board library (add url to preferences; install via board manager)
// esp8266 board library (add url to preferences; install via board manager)
// Wifimanager https://github.com/edumeneses/WiFi32Manager
// Adafruit_LSM9DS1 (library manager)
// ArduinoJSON (library manager - need version 5.13.5; 6.x will not work)
// OSC - https://github.com/CNMAT/OSC (library manager)

//  OBS:
//  1-) Use esp32 1.0.1 or newer (https://github.com/espressif/arduino-esp32/releases)
//  2-) For esp32 1.0.2 ESP32 has problems saving SSID. Follow instructions: https://github.com/edumeneses/WiFi32Manager
//  2-) Also install ESP8266 board library even if you'll use the ESP32 (https://github.com/esp8266/Arduino)
//  3-) Some used library doesn't allow the creation of functions with "setup" in the name
//  4-) Board currently in use: LOLIN D32 PRO
//  5-) Dont forget to change T-Stick Specific Definitions

#include <FS.h>  // this needs to be first, or it all crashes and burns...

#define ESP32; // define ESP8266 or nothing (to use ESP32) 
//#define ESP8266;

#if defined(ESP32)
#include "SPIFFS.h"
#endif

/* You only need to format SPIFFS the first time you run a
   test or else use the SPIFFS plugin to create a partition
   https://github.com/me-no-dev/arduino-esp32fs-plugin */
//#define FORMAT_SPIFFS_IF_FAILED true

#include <WiFi32Manager.h> // https://github.com/edumeneses/WiFi32Manager
// already includes:
// Wifi.h (https://github.com/esp8266/Arduino) or ESP8266WiFi.h (https://github.com/esp8266/Arduino)
// AND
// WebServer.h or ESP8266WebServer.h
// AND
// DNSServer.h

//#include "src/DNSServer/DNSServer.h"
//#include "src/WebServer/WebServer.h"

#include <ArduinoJson.h>  // https://github.com/bblanchon/ArduinoJson
// Wifi.h requires ArduinoJson 5.13.5

#include <Wire.h>
#include <WiFiUdp.h>

// https://github.com/CNMAT/OSC
#include <OSCMessage.h>
#include <OSCBundle.h>

#include <SPI.h>
#include <Adafruit_LSM9DS1.h> // https://github.com/adafruit/Adafruit_LSM9DS1
#include <Adafruit_Sensor.h> // https://github.com/adafruit/Adafruit_Sensor


//////////////////////////////////
//////////////////////////////////
// T-Stick Specific Definitions //
//////////////////////////////////
//////////////////////////////////


unsigned int infoTstick[2] = {190, 19051};    // serial number and firmware revision
char author[20] = "IDMIL"; // DON'T FORGET TO CHANGE ALSO IN WiFi.ino, LINE 132
char nickname[10] = "SN_color"; // sopranino (SN), soprano (SO), tenor (TE), etc.
char APpasswd[15] = "mappings"; // standard AP password

char device[25] = "T-Stick_";
char APpasswdValidate[15];
char APpasswdTemp[15];
int directSendOSC = 0;
char directSendOSCCHAR[3] = "0";
char infoTstickCHAR0[6]; // same serial # to be saved in json
char infoTstickCHAR1[6]; // same firmware # to be saved in json
int calibrate = 0;
char calibrateCHAR[5] = "0"; // same calibrate # to be saved in json
char stored_ssid[20];
char stored_psk[20];

// Debug & calibration definitions
#define DEBUG true
#define CALIB false


//////////////////////
// WiFi Definitions //
//////////////////////

//define your default values here, if there are different values in config.json, they are overwritten.
IPAddress oscEndpointIP(192, 168, 5, 1); // remote IP of your computer
unsigned int oscEndpointPORT = 57120; // remote port to receive OSC
char oscIP[17] = "192.168.10.1";
char oscPORT[7] = "8000";
int timeout1 = 5000; bool timeout1check = false;
WiFiUDP oscEndpoint;            // A UDP instance to let us send and receive packets over UDP
const unsigned int portLocal = 8888;       // local port to listen for OSC packets (actually not used for sending)
bool udpConnected = false;
bool sendOSC = true;
static int bufferFromHost[4] = {0, 0, 0, 0};
int interTouch[2];
char zero[3] = "0";
char one[3] = "1";


//////////////////////////
// LSM9DS1 Library Init //
//////////////////////////

// Adafruit library
Adafruit_LSM9DS1 lsm = Adafruit_LSM9DS1();

float deltat = 0.0f;        // integration interval for both filter schemes
uint32_t lastUpdate = 0;    // used to calculate integration interval
uint32_t Now = 0;           // used to calculate integration interval
float q[4] = {1.0f, 0.0f, 0.0f, 0.0f};    // vector to hold quaternion
float pitch, yaw, roll, heading;
float outAccel[3] = {0, 0, 0};
float outGyro[3] = {0, 0, 0};
float outMag[3] = {0, 0, 0};

uint32_t lastUpdateQuat = 0;    // used to calculate integration interval
uint32_t NowQuat = 0;           // used to calculate integration interval

// global constants for 9 DoF fusion and AHRS (Attitude and Heading Reference System)
#define GyroMeasError PI * (40.0f / 180.0f)       // gyroscope measurement error in rads/s (shown as 3 deg/s)
#define GyroMeasDrift PI * (0.0f / 180.0f)      // gyroscope measurement drift in rad/s/s (shown as 0.0 deg/s/s)
// There is a tradeoff in the beta parameter between accuracy and response speed.
// In the original Madgwick study, beta of 0.041 (corresponding to GyroMeasError of 2.7 degrees/s) was found to give optimal accuracy.
// However, with this value, the LSM9SD0 response time is about 10 seconds to a stable initial quaternion.
// Subsequent changes also require a longish lag time to a stable output, not fast enough for a quadcopter or robot car!
// By increasing beta (GyroMeasError) by about a factor of fifteen, the response time constant is reduced to ~2 sec
// I haven't noticed any reduction in solution accuracy. This is essentially the I coefficient in a PID control sense;
// the bigger the feedback coefficient, the faster the solution converges, usually at the expense of accuracy.
// In any case, this is the free parameter in the Madgwick filtering and fusion scheme.
#define beta sqrt(3.0f / 4.0f) * GyroMeasError   // compute beta
#define zeta sqrt(3.0f / 4.0f) * GyroMeasDrift   // compute zeta, the other free parameter in the Madgwick scheme usually set to a small or zero value

//float pitch, yaw, roll, heading;
float abias[3] = {0, 0, 0}, gbias[3] = {0, 0, 0};
float q1[4] = {0.0f, 0.0f, 0.0f, 0.0f}; // vector to tare the systems of coordinate
float R[3][3] = {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}};

////////////////////////////
// Sketch Output Settings //
////////////////////////////

#define PRINT_CALCULATED
//#define PRINT_RAW
#define PRINT_SPEED 20 // 250 ms between prints
static unsigned long lastPrint = 0; // Keep track of print time

// Earth's magnetic field varies by location. Add or subtract
// a declination to get a more accurate heading. Calculate
// your's here:
// http://www.ngdc.noaa.gov/geomag-web/#declination
#define DECLINATION -14.34 // Declination (degrees) in Montreal, QC.


//////////////
// defaults //
//////////////

int piezoPin = 32;
int pressurePin = 33;
int ledPin = 5; //changed for The thing dev during testing
int ledStatus = 0;
int ledTimer = 1000;
byte touch[2] = {0, 0};
int touchMask[2] = {255, 255};
char touchMaskCHAR0[7] = "255"; // same cal # to be saved in json
char touchMaskCHAR1[7] = "255"; // same cal # to be saved in json
unsigned int calibrationData[2] = {0, 1024};
char calibrationDataCHAR0[6] = "0"; // same cal # to be saved in json
char calibrationDataCHAR1[6] = "1024"; // same cal # to be saved in json
unsigned int pressure = 0;
const int buttonPin = 15;



////////////////////////
//control definitions //
////////////////////////

unsigned long then = 0;
unsigned long now = 0;
unsigned long started = 0;
unsigned long lastRead = 0;
byte interval = 10;
byte touchInterval = 15;
unsigned long lastReadSensors = 0;
int buttonState = 0;         // variable for reading the pushbutton status


void setup() {

  
  //wifiManager.setDebugOutput(true);

  Serial.begin(115200);
  if (DEBUG == true) {
    Serial.println("\n Starting");
  }

  // Converting T-Stick info into char (str)
  itoa(infoTstick[0], infoTstickCHAR0, 10);
  itoa(infoTstick[1], infoTstickCHAR1, 10);
  strcat(device, nickname);
  strcat(device, "_");
  strcat(device, infoTstickCHAR0);
  memcpy(APpasswdValidate, APpasswd, 15);
  memcpy(APpasswdTemp, APpasswd, 15);
  
  // Starting WiFiManager in Trigger Mode
  Wifimanager_init(DEBUG);

  #if defined(ESP8266)
  WiFi.hostname(device);
  #else
  WiFi.setHostname(device);
  #endif

  initIMU();

  initCapsense();

  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  delay(100);

  if (DEBUG == true) {
    Serial.println("Setup complete.");
  }

  #if defined(ESP32) 
    if (WiFi.status() != WL_CONNECTED) {
      if (DEBUG == true) {Serial.println("ESP32 3rd attempt to connect");}
      WiFi.begin(stored_ssid, stored_psk);
      if (DEBUG == true) {
        if (WiFi.status() != WL_CONNECTED) {Serial.println("Connected on 3rd attempt");}
        else {Serial.println("Failed to connect, finishing setup anyway");}
      }
    }
  #endif
  
}

void loop() {

  
  // Calling WiFiManager configuration portal
  buttonState = digitalRead(buttonPin);
  //if ( outAccel[0] > 1 && interTouch[0] == 9 && interTouch[1] == 144 ) {
  if ( buttonState == LOW ) {
    digitalWrite(ledPin, HIGH);
    Wifimanager_portal(device, APpasswd, true, DEBUG);
  }

  byte dataRec = OSCMsgReceive();

  if (dataRec) { //Check for OSC messages from host computers
    if (DEBUG) {
      Serial.println();
      for (int i = 0; i < 4; i++) {
        Serial.printf("From computer %d: ", i); Serial.println(bufferFromHost[i]);
      }
      Serial.println();
    }
    char message = bufferFromHost[0];

    OSCMessage msg0("/information");

    switch (message) {
      case 's': // start message,
        msg0.add(infoTstick[0]);
        msg0.add(infoTstick[1]);
        oscEndpoint.beginPacket(oscEndpointIP, oscEndpointPORT);
        msg0.send(oscEndpoint);
        oscEndpoint.endPacket();
        msg0.empty();

        started = millis();
        break;
      case 'x': // stop message,
        started = 0;
        break;
      case 'c': // calibrate message
        switch (bufferFromHost[1]) {
          case 1: // FSR calibration
            calibrationData[0] = bufferFromHost[2];
            calibrationData[1] = bufferFromHost[3];
            calibrate = 1;
            bufferFromHost[1] = 0;
            bufferFromHost[2] = 0;
            bufferFromHost[3] = 0;
            break;
          default:
            calibrate = 0;
            break;
        }
        break;
      case 'w':     //write settings
        switch ((char)bufferFromHost[1]) {
          case 'i': //write info
            infoTstick[0] = bufferFromHost[2];
            infoTstick[1] = bufferFromHost[3];
            bufferFromHost[1] = 0;
            bufferFromHost[2] = 0;
            bufferFromHost[3] = 0;
            break;
          case 'T': //write touch mask
            touchMask[0] = bufferFromHost[2];
            touchMask[1] = bufferFromHost[3];
            bufferFromHost[1] = 0;
            bufferFromHost[2] = 0;
            bufferFromHost[3] = 0;
            break;
          case 'w': // write settings to memory (json)
            save_to_json(DEBUG);
            bufferFromHost[1] = 0;
            break;
          case 'r': // sending the config info trough OSC
            msg0.add(infoTstick[0]);
            msg0.add(infoTstick[1]);
            msg0.add(calibrate);
            msg0.add(calibrationData[0]);
            msg0.add(calibrationData[1]);
            msg0.add(touchMask[0]);
            msg0.add(touchMask[1]);
            oscEndpoint.beginPacket(oscEndpointIP, oscEndpointPORT);
            msg0.send(oscEndpoint);
            oscEndpoint.endPacket();
            msg0.empty();
            bufferFromHost[1] = 0;
            break;
          default:
            break;
        }
      default:
        break;
    }
  }

  now = millis();

  if (directSendOSC == 0) {
    if (now < started + 2000) { // needs a ping/keep-alive every 2 seconds or less or will time-out
      TStickRoutine();
    }
    else { // runs when there's no ping/keep-alive
      now = millis();

      // Calling configuration portal if the T-Stick is disconnected
//      if (millis() - lastRead > interval) {
//        lastRead = millis();
//        pressure = analogRead(pressurePin);
//        //if (DEBUG) {Serial.print("Pressure sensor: "); Serial.println(pressure);}
//      }
      //if ( pressure > 4090 ) {
      if ( buttonState == LOW ) {
        digitalWrite(ledPin, HIGH);
        Wifimanager_portal(device, APpasswd, true, DEBUG);
      }
      else {
        if ((now - then) > ledTimer) {
          ledBlink();
          then = now;
        }
        else if ( then > now) {
          then = 0;
        }
      }
    }
  } else {
    TStickRoutine();
  }

} // END LOOP

void ledBlink() {
  ledStatus = (ledStatus + 1) % 2;
  digitalWrite(ledPin, ledStatus);
}
