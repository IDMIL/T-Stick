//********************************************************************************//
//  Sopranino T-Stick 2GW - Wemos D1 - USB -WiFi                                  //
//  Input Devices and Music Interaction Laboratory                                //
//  Created:  February 2018 by Alex Nieva                                         //
//            February 2019 by Edu Meneses - version 19021 (2019-02.v1)           //
//  Notes   : Based on test program for reading CY8C201xx using I2C               //
//            by Joseph Malloch 2011                                              //
//                                                                                //
//            Adapted to work with Arduino IDE 1.8.5 and T-Stick Sopranino 2GW    //
//********************************************************************************//
// See the CY8C201xx Register Reference Guide for more info:
// http://www.cypress.com/?docID=24620
// This chip is outdated now, see newer designs.

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
// Edu Meneses - IDMIL - Feb 2019                   //
//**************************************************//

// FIRMWARE VERSION: 19021

//  OBS:
//  1-) Use esp32 1.0.1 or newer (https://github.com/espressif/arduino-esp32/releases)
//  2-) Some used library doesn't allow the creation of functions with "setup" in the name


#include <FS.h>  // this needs to be first, or it all crashes and burns...

#define ESP8266; // define ESP8266 or nothing (to use ESP32) 

#if defined(ESP8266)
#else
#include "SPIFFS.h"
#include <HardwareSerial.h>

HardwareSerial Serial1(1);

#define SERIAL1_RXPIN 32
#define SERIAL1_TXPIN 33
#endif

#include <WiFi32Manager.h> // https://github.com/edumeneses/WiFi32Manager
// already includes:
// Wifi.h (https://github.com/esp8266/Arduino) or ESP8266WiFi.h (https://github.com/esp8266/Arduino)
// AND
// WebServer.h or ESP8266WebServer.h
// AND
// DNSServer.h

#include <ArduinoJson.h>  // https://github.com/bblanchon/ArduinoJson

// include Wire library to read and write I2C commands:
#include <Wire.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>

// The SFE_LSM9DS0 requires both the SPI and Wire libraries.
// Unfortunately, you'll need to include both in the Arduino
// sketch, before including the SFE_LSM9DS0 library.
#include <SPI.h> // Included for SFE_LSM9DS0 library
#include <SFE_LSM9DS0.h>

//////////////////////////////////
// T-Stick Specific Definitions //
//////////////////////////////////
char device[15] = "T-Stick_181";
char APpasswd[15] = "mappings";
char APpasswdValidate[15] = "mappings";
char APpasswdTemp[15] = "mappings";
int directSendOSC = 0;
char directSendOSCCHAR[3] = "0";

#define DEBUG false
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

///////////////////////
//  IMU Definitions  //
///////////////////////
// Comment out this section if you're using SPI
// SDO_XM and SDO_G are both grounded, so our addresses are:
#define LSM9DS0_XM  0x1D // Would be 0x1E if SDO_XM is LOW
#define LSM9DS0_G   0x6B // Would be 0x6A if SDO_G is LOW
// Create an instance of the LSM9DS0 library called `dof` the
// parameters for this constructor are:
// [SPI or I2C Mode declaration],[gyro I2C address],[xm I2C add.]
LSM9DS0 dof(MODE_I2C, LSM9DS0_G, LSM9DS0_XM);

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

// IMU buffers and intervals
float rawAccel[3] = {0, 0, 0};
float rawGyro[3] = {0, 0, 0};
float rawMag[3] = {0, 0, 0};
//Rotated variables
float outAccel[3] = {0, 0, 0};
float outGyro[3] = {0, 0, 0};
float outMag[3] = {0, 0, 0};

//float pitch, yaw, roll, heading;
float abias[3] = {0, 0, 0}, gbias[3] = {0, 0, 0};
float q[4] = {1.0f, 0.0f, 0.0f, 0.0f};    // vector to hold quaternion
float q1[4] = {0.0f, 0.0f, 0.0f, 0.0f}; // vector to tare the systems of coordinate
float R[3][3] = {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}};
float temperature;
bool IMUcalibrated = false;
bool rotateManually = true;

float deltat = 0.0f;        // integration interval for both filter schemes
uint32_t lastUpdateQuat = 0;    // used to calculate integration interval
uint32_t NowQuat = 0;           // used to calculate integration interval

uint32_t AccelDeltaRead = 5000; //microseconds
uint32_t PreviousAccelRead = 0;
uint32_t GyroDeltaRead = 5263; //microseconds
uint32_t PreviousGyroRead = 0;
uint32_t MagDeltaRead = 80000; //microseconds
uint32_t PreviousMagRead = 0;
uint32_t dataTransferRate = 50; // sending data at 20Hz
uint32_t deltaTransferRate = 0;
unsigned int pressure = 0;
unsigned int calpressure = 0;

// defaults
unsigned int infoTstick[2] = {181, 19021};    // serial number and firmware revision
char infoTstickCHAR0[6] = "181"; // same serial # to be saved in json
char infoTstickCHAR1[6] = "19021"; // same firmware # to be saved in json
int xres = D5;  // XRES pin on one of the CY8C201xx chips is connected to Arduino pin 13
int xres1 = D0;
int pressurePin = A0;
int ledPin = D3;
int ledStatus = 0;
int ledTimer = 1000;
byte touch1 = 0;
byte touch[2] = {0, 0};
int touchMask[2] = {255, 255};
char touchMaskCHAR0[7] = "255"; // same cal # to be saved in json
char touchMaskCHAR1[7] = "255"; // same cal # to be saved in json
unsigned int calibrationData[2] = {0, 1024};
char calibrationDataCHAR0[6] = "0"; // same cal # to be saved in json
char calibrationDataCHAR1[6] = "1024"; // same cal # to be saved in json

//control definitions
unsigned long then = 0;
unsigned long now = 0;
unsigned long started = 0;
int calibrate = 0;
char calibrateCHAR[5] = "0"; // same calibrate # to be saved in json

// I2C adresses CY8C201xx
#define I2C_ADDR0 0x00
#define I2C_ADDR0_NOPULL 0x80
#define I2C_ADDR1 0x02
#define I2C_ADDR1_NOPULL 0x82

// some CY8C201xx registers
#define INPUT_PORT0 0x00
#define INPUT_PORT1 0x01
#define CS_ENABLE0 0x06
#define CS_ENABLE1 0x07
#define I2C_DEV_LOCK 0x79
#define I2C_ADDR_DM 0x7C
#define COMMAND_REG 0xA0

// Secret codes for locking/unlocking the I2C_DEV_LOCK register
byte I2CDL_KEY_UNLOCK[3] = {0x3C, 0xA5, 0x69};
byte I2CDL_KEY_LOCK[3] = {0x96, 0x5A, 0xC3};

unsigned long lastRead = 0;
int interval = 15;

void setup() {

  Serial.begin(115200);
  if (DEBUG == true) {
    Serial.println("\n Starting");
  }

  // Starting WiFiManager in Trigger Mode
  Wifimanager_init(DEBUG);

#if defined(ESP8266)
  WiFi.hostname(device);
#else
  WiFi.setHostname(device);
#endif

  //Configure I2C bus
  I2Cinit();

  pinMode(ledPin, OUTPUT);

  delay(100);

  //  if(DEBUG==true){
  //    if (WiFi.status() == WL_CONNECTED) {
  //      Serial.println("Connected to the saved network");
  //      Serial.print("Connected to: "); Serial.println(WiFi.SSID());
  //      Serial.print("IP address: "); Serial.println(WiFi.localIP()); // Print Ip on serial monitor or any serial debugger
  //      WiFi.printDiag(Serial); // Remove this line if you do not want to see WiFi password printed
  //      delay(100);
  //      // Starting UDP
  //      //oscEndpoint.begin(portLocal);
  //      udpConnected = connectUDP();
  //    }
  //    else {
  //      Serial.println("Failed to connect, finishing setup anyway");
  //      Serial.print("Saved SSID: "); Serial.println(WiFi.SSID());
  //      WiFi.printDiag(Serial); // Remove this line if you do not want to see WiFi password printed
  //    }
  //  }
  //
  //  delay(100);

  if (DEBUG == true) {
    Serial.println("Setup complete.");
  }
}

void loop() {

  // Calling WiFiManager configuration portal
  if ( outAccel[0] > 1 && interTouch[0] == 9 && interTouch[1] == 144 ) {
    Wifimanager_portal(device, APpasswd, true, DEBUG);
  }

  byte dataRec = OSCMsgReceive();

  if (dataRec) { //Check for OSC messages from host computers
    if (DEBUG) {
      Serial.println();
      for (int i = 0; i < 4; i++) {
        Serial.printf("From Max %d: ", i); Serial.println(bufferFromHost[i]);
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
      if (millis() - lastRead > interval) {
        lastRead = millis();
        pressure = analogRead(pressurePin);
      }
      if ( pressure > 1000 ) {
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

byte readTouch(int address) {
  byte touch = 0;
  // request Register 00h: INPUT_PORT0
  Wire.beginTransmission(address);
  Wire.write(INPUT_PORT0);
  Wire.endTransmission();

  Wire.requestFrom(address, 1);
  while (Wire.available()) {
    touch = Wire.read() << 4;
  }
  // request Register 01h: INPUT_PORT1
  Wire.beginTransmission(address);
  Wire.write(INPUT_PORT1);
  Wire.endTransmission();

  Wire.requestFrom(address, 1);
  while (Wire.available()) {
    touch |= Wire.read();
  }

  byte touch_temp = 0;

  for (int i = 0; i < 8; i++) {
    byte mult = 1 << i;
    byte temp = ((touch & mult) >> i) % 2;
    switch (i) {
      case 0:
        touch_temp |= temp << 7;
        break;
      case 1:
        touch_temp |= temp << 4;
        break;
      case 2:
        touch_temp |= temp << 5;
        break;
      case 3:
        touch_temp |= temp << 2;
        break;
      case 4:
        touch_temp |= temp << 1;
        break;
      case 5:
        touch_temp |= temp << 6;
        break;
      case 6:
        touch_temp |= temp << 3;
        break;
      case 7:
        touch_temp |= temp << 0;
        break;
    }
  }

  touch = touch_temp;

  return touch;

}
