#pragma once

#define SOPRANINO
// #define SOPRANO

#include "platforms/platforms.h"

#include <Arduino.h>

#include <FS.h>

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

#include <stdlib.h> //floats to string

#include <mpr/mpr.h>

#define BUTTON_STAT 0xAA  // Address to read the status of the sensors (2 bytes)

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
  byte touchMask[5][2];
  float abias[3];
  float mbias[3];
  float gbias[3];
  float acclcalibration[9];
  float magncalibration[9];
  float gyrocalibration[9];
};

struct RawDataStruct {
  byte touch[5][2]; // /raw/capsense, i..., 0--255, ... (1 int per 8 capacitive stripes -- 8 bits)
  int fsr; // /raw/fsr, i, 0--4095
  int piezo; // /raw/piezo, i, 0--1023
  float accl[3]; // /raw/accl, iii, +/-32767 (integers)
  float gyro[3]; // /raw/gyro, fff, +/-34.90659 (floats)
  float magn[3]; // /raw/magn, fff, +/-32767 (integers)
  float raw[10]; // /raw (IMU data to be send to callibration app)
  float quat[4]; // /raw/quat, ffff, ?, ? ,? ,?
  float ypr[3]; // /raw/ypr, fff, +/-180, +/-90 ,+/-180 (degrees)
  float magAccl;
  float magGyro;
  float magMagn;
  byte buttonShort; // /raw/button/short, i, 0 or 1
  byte buttonLong; // /raw/button/long, i, 0 or 1
  byte buttonDouble; // /raw/button/double, i, 0 or 1
};

struct NormDataStruct {
  float fsr; // /norm/fsr, f, 0--1
  float piezo; // /norm/piezo, f, 0--1
  float accl[3]; // /norm/accl, fff, +/-1, +/-1, +/-1
  float gyro[3]; // /norm/gyro, fff, +/-1, +/-1, +/-1
  float magn[3]; // /norm/magn, fff, +/-1, +/-1, +/-1
};

struct LastStateDataStruct {
  int blobPos[4];
  int gyroArrayCounter;
  float gyroXArray[5];
  float gyroYArray[5];
  float gyroZArray[5];
};

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
};

struct Capsense { 
  byte answer1, answer2;
};


extern Tstick Tstick;
extern RawDataStruct RawData;
extern NormDataStruct NormData;
extern LastStateDataStruct LastStateData;
extern InstrumentDataStruct InstrumentData;
extern Capsense capsense;

//////////////////////////////////
//////////////////////////////////
// T-Stick Specific Definitions //
//////////////////////////////////
//////////////////////////////////

extern IPAddress osc_IP; // used to send OSC messages
extern char APpasswdTemp[15]; // used to check before save new T-Stick passwd
extern char APpasswdValidate[15]; // used to check before save new T-Stick passwd
extern char tstickSSID[30];
extern char tstickID[5];
extern float battery;
extern byte batteryPercentage;
extern int batteryInterval;
extern unsigned long  batteryLastRead;
extern unsigned long  batteryLastSend;
extern byte batteryCount;

//////////////////////
// WiFi Definitions //
//////////////////////

extern WiFiUDP oscEndpoint; // A UDP instance to let us send and receive packets over UDP
extern const unsigned int portLocal; // local port to listen for OSC packets (not used for sending)

///////////////
// blink LED //
///////////////

#ifdef TSTICK193
      extern const int ledPin;
  #else
      extern const int ledPin;
#endif

extern bool ledStatus;
extern int long_blink;
extern int short_blink;
extern int flickering;

extern MIMU_LSM9DS1 mimu; // use default SDA and SCL as per board library macros
extern MIMUCalibrator calibrator;
extern MIMUFusionFilter filter;

extern byte capsense_addresses[4];
extern byte nCapsenses;
extern byte touch[5][2]; // up to 5 capsenses (2 bytes per capsense)

extern byte piezoPin;
extern byte fsrPin;
extern byte batteryPin;
extern const int buttonPin;
extern byte buttonState;
extern const int buttonShortInterval;
extern const int buttonLongInterval;
extern unsigned long buttonTimer;
extern byte buttonShortFlag;
extern byte buttonLongFlag;
extern unsigned long buttonDoubleTimer;
extern byte buttonDoubleFlag;
extern int waitForConnection;
extern int serialInterval;
extern unsigned long serialLastRead;
extern unsigned long time_now;

extern float fsrbuf;

extern float offsetYaw;
extern unsigned long offsetDebounce;
extern byte offsetFlag;

extern mpr_dev libmapper_dev;

/////////////////////////
// Function prototypes //
/////////////////////////

void blinkLED(int ledInterval);
void receiveTouchMask(OSCMessage &msg);
void sendInfo(OSCMessage &msg);
void processJson(OSCMessage &msg);
void receiveFSRoffset(OSCMessage &msg);
void openPortalOSC(OSCMessage &msg);
void saveIMUaVector(OSCMessage &msg);
void saveIMUaMatrix(OSCMessage &msg);
void Wifimanager_portal(char *portal_name, char *portal_password);
void saveJSON();
void parseJSON();
void saveIMUmVector(OSCMessage &msg);
void saveIMUmMatrix(OSCMessage &msg);
void saveIMUgVector(OSCMessage &msg);
void saveIMUgMatrix(OSCMessage &msg);
void createTstickSSID();
Capsense capsenseRequest(uint8_t address,uint8_t request, uint8_t answer_size);
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max);
void copyFloatArrayToVar(const float source[], int size, float destination[]);
void taitBryanAngles(float w, float x, float y, float z);
void mountFS();
void printJSON();
void printVariables();
void connectToWifi();
void initIMU();
void capsense_scan();
void initLibmapper();
void readData();
void sendOSC(char* ip,int32_t port);
void updateLibmapper();
void receiveOSC();
void printData();

void setup();
void loop();
