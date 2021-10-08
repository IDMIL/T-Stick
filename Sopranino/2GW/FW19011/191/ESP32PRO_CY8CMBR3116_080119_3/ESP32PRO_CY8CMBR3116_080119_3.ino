/*
 * Sopranino T-Stick 2GW Fall 2018 - Winter 2019 version
 * Alex Nieva Fall 2018 - Winter 2019
 * Input Devices and Music Interaction Laboratory - McGill University
 * http://www-new.idmil.org/project/the-t-stick/
 * 
 * Based on the original T-Stick design by Joseph Malloch
 * 
 * This code is open-source
 * 
 */

// WiFi ESP32
#include <WiFi.h>
#include <WiFiUdp.h>

#include <OSCMessage.h>
 
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_LSM9DS1.h>
#include <Adafruit_Sensor.h>  // not used in this demo but required!

// EEPROM library
#include <EEPROM.h>
#define min(X, Y) (((X)<(Y))?(X):(Y))
#define max(X, Y) (((X)>(Y))?(X):(Y))


// Capsense Definitions
#define SENSOR_EN 0x00
#define FSS_EN 0x02
#define SENSITIVITY0 0x08
#define SENSITIVITY1 0x09
#define SENSITIVITY2 0x0A
#define SENSITIVITY3 0x0B
#define DEVICE_ID 0x90    // Should return 0xA05 (returns 2 bytes)
#define FAMILY_ID 0x8F
#define SYSTEM_STATUS 0x8A
#define I2C_ADDR 0x37     // Should return 0x37
#define REFRESH_CTRL 0x52
#define SENSOR_EN 0x00    // We should set it to 0xFF for 16 sensors
#define BUTTON_STAT 0xAA  // Here we red the status of the sensors (2 bytes)
#define CTRL_CMD 0x86     // To configure the Capsense
#define CTRL_CMD_STATUS 0x88
#define CTRL_CMD_ERROR 0x89
#define BUTTON_LBR 0x1F
#define SPO_CFG 0x4C      //CS15 configuration address
#define GPO_CFG 0x40
#define CALC_CRC 0x94
#define CONFIG_CRC 0x7E

// Debug & calibration definitions
#define DEBUG true
#define CALIB false

int deviceID = 0;

//control definitions
unsigned long then = 0;
unsigned long now = 0;
unsigned long started = 0;
unsigned long lastRead = 0;
byte interval = 10;
byte touchInterval = 15;
byte calibrate = 0;
unsigned long lastReadSensors = 0;

// defaults
unsigned int infoTstick[2] = {0, 0};    // serial number and firmware revision
int piezoPin = 32;
int pressurePin = 33;
int ledPin = 12; //changed for The thing dev during testing
int ledStatus = 0;
int ledTimer = 1000;
byte touch[2] = {0, 0};
byte touchMask[2] = {255, 255};
unsigned int calibrationData[2] = {0, 4095};

uint32_t dataTransferRate = 20; // sending data at 50Hz
uint32_t deltaTransferRate = 0;

// the current address in the EEPROM (i.e. which byte
// we're going to write to next)
int addr = 0;             // Start of instrument info
int addrNet = addr + 12;  // Start of Network info

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


///////////////////////////
// WiFi & OSC settings
//////////////////////////
// WiFi Network variables
const char *ssid = "SopraninoWiFi-191";           //Ap SSID
const char *password = "password";                //Ap Password

bool sendOSC = true;
WiFiUDP oscEndpoint;            // A UDP instance to let us send and receive packets over UDP
IPAddress oscEndpointIP;        // remote IP - your computer
int oscEndpointPORT;            // remote port to receive OSC
const unsigned int portLocal = 8888;   
static int bufferFromHost[4] = {0, 0, 0, 0};
  
//// Set web server port number to 80
//WiFiServer server(80);
//
//// Variable to store the HTTP request
//String header;

void setup() {

  setupSerial();

  setupIMU();
  
  setupCapsense();
  
  setupWiFi();

  EEPROM.begin(512);
  Serial.println();

  readSettings();

  pinMode(ledPin, OUTPUT);
  
}

void loop() {
  byte dataRec = OSCMsgReceive();

  if (dataRec){ //Check for OSC messages from host computers
    if (DEBUG){
      Serial.println();
      for (int i = 0; i<4; i++){
      Serial.printf("From Max %d: ", i); Serial.println(bufferFromHost[i]);
      }  
      Serial.println();
    }
    char message = bufferFromHost[0];
    OSCMessage msg0("/information");

    switch (message){
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
        switch(bufferFromHost[1]){
          case 1: // FSR calibration
            calibrationData[0] = 4095;
            calibrationData[1] = 0;
            calibrate = 1;
            bufferFromHost[1] = 0;            
            break;
          default:
            calibrate = 0;
            break;
        }
        break;
      case 'w':     //write settings
        switch((char)bufferFromHost[1]){
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
          case 'w':
            writeSettings();
            bufferFromHost[1] = 0;            
            break;
          case 'r': 
            readSettings();
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

  if (now < started + 2000){ // needs a ping/keep-alive every 2 seconds or less or will time-out
    if (sendOSC){
      if (millis()-lastRead>touchInterval) {
        lastRead = millis();
        if (readTouch()){
          OSCMessage msg1("/rawcapsense");
          msg1.add((int)touch[0] & touchMask[0]);
          msg1.add((int)touch[1] & touchMask[1]);
          oscEndpoint.beginPacket(oscEndpointIP, oscEndpointPORT);
          msg1.send(oscEndpoint);
          oscEndpoint.endPacket();
          msg1.empty();
        }
      }
    
      if ((millis() - deltaTransferRate) > dataTransferRate){

        lsm.read();  /* ask it to read in the data */ 
      
        /* Get a new sensor event */ 
        sensors_event_t a, m, g, temp;
        //sensor_t aRaw, mRaw, gRaw, tempRaw;
      
        lsm.getEvent(&a, &m, &g, &temp); 
        //lsm.getSensor(&aRaw, &mRaw, &gRaw, &tempRaw);
        
        outAccel[0] = a.acceleration.x / 9.80665F;
        outAccel[1] = a.acceleration.y / 9.80665F;
        outAccel[2] = a.acceleration.z / 9.80665F;
        
        outMag[0] = m.magnetic.x;
        outMag[1] = m.magnetic.y;
        outMag[2] = m.magnetic.z;
        
        outGyro[0] = g.gyro.x;
        outGyro[1] = g.gyro.y;
        outGyro[2] = g.gyro.z;
        
  
        OSCMessage msg2("/rawgyro");
        msg2.add(outGyro[0]);
        msg2.add(outGyro[1]);
        msg2.add(outGyro[2]);
        oscEndpoint.beginPacket(oscEndpointIP, oscEndpointPORT);
        msg2.send(oscEndpoint);
        oscEndpoint.endPacket();
        msg2.empty();
    
        OSCMessage msg3("/rawaccel");
        msg3.add(outAccel[0]);
        msg3.add(outAccel[1]);
        msg3.add(outAccel[2]);
        oscEndpoint.beginPacket(oscEndpointIP, oscEndpointPORT);
        msg3.send(oscEndpoint);
        oscEndpoint.endPacket();
        msg3.empty();
    
        OSCMessage msg4("/rawmag");
        msg4.add(outMag[0]);
        msg4.add(outMag[1]);
        msg4.add(outMag[2]);
        oscEndpoint.beginPacket(oscEndpointIP, oscEndpointPORT);
        msg4.send(oscEndpoint);
        oscEndpoint.endPacket();
        msg4.empty();
    
        unsigned int pressure = analogRead(pressurePin);
//        if (calibrate == 1) {
//          calibrationData[0] = constrain(min(calibrationData[0], pressure), 0, 4095);
//          calibrationData[1] = constrain(max(calibrationData[1], pressure), 0, 4095);
//        }
//        pressure = constrain(map(pressure, calibrationData[0], calibrationData[1], 0, 4095), 0, 4095);
        OSCMessage msg5("/rawpressure");
        msg5.add(pressure);
        oscEndpoint.beginPacket(oscEndpointIP, oscEndpointPORT);
        msg5.send(oscEndpoint);
        oscEndpoint.endPacket();
        msg5.empty();
        deltaTransferRate = millis();

        unsigned int piezo = analogRead(piezoPin);
//        if (calibrate == 1) {
//          calibrationData[0] = constrain(min(calibrationData[0], piezo), 0, 4095);
//          calibrationData[1] = constrain(max(calibrationData[1], piezo), 0, 4095);
//        }
//        piezo = constrain(map(piezo, calibrationData[0], calibrationData[1], 0, 4095), 0, 4095);
        OSCMessage msg6("/rawpiezo");
        msg6.add(piezo);
        oscEndpoint.beginPacket(oscEndpointIP, oscEndpointPORT);
        msg6.send(oscEndpoint);
        oscEndpoint.endPacket();
        msg6.empty();
        deltaTransferRate = millis();
          
      }
    }  
    ledBlink();
    then = now;
  }
  else {
    now = millis();
    if ((now - then) > ledTimer){
      ledBlink();
      then = now;
    }
    else if ( then > now)
      then = 0;
  }
    
//    lsm.read();  /* ask it to read in the data */ 
//  
//    /* Get a new sensor event */ 
//    sensors_event_t a, m, g, temp;
//    //sensor_t aRaw, mRaw, gRaw, tempRaw;
//  
//    lsm.getEvent(&a, &m, &g, &temp); 
//    //lsm.getSensor(&aRaw, &mRaw, &gRaw, &tempRaw);
//  
////    Serial.print("Accel X: "); Serial.print(a.acceleration.x); Serial.print(" m/s^2");
////    Serial.print("\tY: "); Serial.print(a.acceleration.y);     Serial.print(" m/s^2 ");
////    Serial.print("\tZ: "); Serial.print(a.acceleration.z);     Serial.println(" m/s^2 ");
////  
////    Serial.print("Mag X: "); Serial.print(m.magnetic.x);   Serial.print(" gauss");
////    Serial.print("\tY: "); Serial.print(m.magnetic.y);     Serial.print(" gauss");
////    Serial.print("\tZ: "); Serial.print(m.magnetic.z);     Serial.println(" gauss");
////  
////    Serial.print("Gyro X: "); Serial.print(g.gyro.x);   Serial.print(" dps");
////    Serial.print("\tY: "); Serial.print(g.gyro.y);      Serial.print(" dps");
////    Serial.print("\tZ: "); Serial.print(g.gyro.z);      Serial.println(" dps");
////  
////    Serial.println(); 
//  
////    Serial.print("Accel X: "); Serial.print(lsm.accelData.x); //Serial.print(" m/s^2");
////    Serial.print("\tY: "); Serial.print(lsm.accelData.y);     //Serial.print(" m/s^2 ");
////    Serial.print("\tZ: "); Serial.println(lsm.accelData.z);     //Serial.println(" m/s^2 ");
////  
////    Serial.print("Mag X: "); Serial.print(lsm.magData.x);   //Serial.print(" gauss");
////    Serial.print("\tY: "); Serial.print(lsm.magData.y);     //Serial.print(" gauss");
////    Serial.print("\tZ: "); Serial.println(lsm.magData.z);     //Serial.println(" gauss");
////  
////    Serial.print("Gyro X: "); Serial.print(lsm.gyroData.x);   //Serial.print(" dps");
////    Serial.print("\tY: "); Serial.print(lsm.gyroData.y);      //Serial.print(" dps");
////    Serial.print("\tZ: "); Serial.println(lsm.gyroData.z);      //Serial.println(" dps");
////  
//    Serial.println(); 
//    
//    printAttitude(a.acceleration.x, a.acceleration.y, a.acceleration.z, m.magnetic.x, m.magnetic.y, m.magnetic.z);
//    Serial.println();
//    
//    Now = micros();
//    deltat = ((Now - lastUpdate)/1000000.0f); // set integration time by time elapsed since last filter update
//    lastUpdate = Now;
//    //Serial.print("deltat = "); Serial.println(deltat,6);
//    
//    MadgwickQuaternionUpdate(lsm.accelData.x, lsm.accelData.y, lsm.accelData.z, lsm.gyroData.x*PI/180.0f, lsm.gyroData.y*PI/180.0f, lsm.gyroData.z*PI/180.0f, lsm.magData.x, lsm.magData.y, lsm.magData.z);
//    Serial.print("q[0]: "); Serial.print(q[0]); Serial.print("\t"); 
//    Serial.print("q[1]: "); Serial.print(q[1]); Serial.print("\t");
//    Serial.print("q[2]: "); Serial.print(q[2]); Serial.print("\t");
//    Serial.print("q[3]: "); Serial.print(q[3]); Serial.println();
//
//    yaw   = atan2(2.0f * (q[1] * q[2] + q[0] * q[3]), q[0] * q[0] + q[1] * q[1] - q[2] * q[2] - q[3] * q[3]);   
//    pitch = -asin(2.0f * (q[1] * q[3] - q[0] * q[2]));
//    roll  = atan2(2.0f * (q[0] * q[1] + q[2] * q[3]), q[0] * q[0] - q[1] * q[1] - q[2] * q[2] + q[3] * q[3]);
//    pitch *= 180.0f / PI;
//    yaw   *= 180.0f / PI; 
//    yaw -= DECLINATION; // Declination at Montreal, QC.-14.49
//    roll  *= 180.0f / PI;
//    Serial.print("Pitch, Roll: ");
//    Serial.print(pitch, 2);
//    Serial.print(", ");
//    Serial.println(roll, 2);
//    Serial.print("Yaw: "); Serial.println(yaw, 2);
//    
//    lastReadSensors = millis();
//  }
}


void ledBlink() {
  ledStatus = (ledStatus + 1) % 2;
  digitalWrite(ledPin, ledStatus);
}


boolean readTouch(){
  boolean changed = 0;
  byte temp[2] = {0, 0}; int i=0;
  Wire.beginTransmission(I2C_ADDR);
  Wire.write(BUTTON_STAT);
  Wire.endTransmission();

  Wire.requestFrom(I2C_ADDR,2);
  while (Wire.available()) { // slave may send less than requested
//    byte c = Wire.read();
//    temp[i] = c; // receive a byte as character
    temp[i] = Wire.read();
    i++;
  }    
  Wire.endTransmission();

  for (int t = 0; t<2; t++){
    if (temp[t] != touch[t]){
      changed = 1;
      touch[t] = temp[t];
    }
  }
  return changed;
}



void readSettings(){
  Serial.println("Reading settings from EEPROM...");
//  Serial.print("byte 0: "); Serial.println(EEPROM.read(addr));
//  Serial.print("byte 1: "); Serial.println(EEPROM.read(addr+1));
//  Serial.print("byte 2: "); Serial.println(EEPROM.read(addr+2));
//  Serial.print("byte 3: "); Serial.println(EEPROM.read(addr+3));
  infoTstick[0] = (unsigned int)(EEPROM.read(addr+1)) + (unsigned int)(EEPROM.read(addr)<<8);
  infoTstick[1] = (unsigned int)(EEPROM.read(addr+3)) + (unsigned int)(EEPROM.read(addr+2)<<8);
  if (DEBUG)
  {
    Serial.print("value infoTstick1: "); Serial.println(infoTstick[0]);
    Serial.print("value infoTstick2: "); Serial.println(infoTstick[1]);
  }
  calibrationData[0] = (unsigned int)(EEPROM.read(addr+5)) + (unsigned int)(EEPROM.read(addr+4)<<8);
  calibrationData[1] = (unsigned int)(EEPROM.read(addr+7)) + (unsigned int)(EEPROM.read(addr+6)<<8);  
}

void writeSettings(){
  Serial.println("Writing settings to EEPROM...");
  EEPROM.write(addr,   (byte)(infoTstick[0]/256));
//  Serial.print("byte 0: "); Serial.println((byte)(infoTstick[0]/256));
  EEPROM.write(addr+1, (byte)(infoTstick[0]%256));
//  Serial.print("byte 1: "); Serial.println((byte)(infoTstick[0]%256));
  EEPROM.write(addr+2, (byte)(infoTstick[1]/256));
//  Serial.print("byte 2: "); Serial.println((byte)(infoTstick[1]/256));
  EEPROM.write(addr+3, (byte)(infoTstick[1]%256));
//  Serial.print("byte 3: "); Serial.println((byte)(infoTstick[1]%256));
  EEPROM.write(addr+4, (byte)(calibrationData[0]/256));
  EEPROM.write(addr+5, (byte)(calibrationData[0]%256));
  EEPROM.write(addr+6, (byte)(calibrationData[1]/256));
  EEPROM.write(addr+7, (byte)(calibrationData[1]%256));
  
  EEPROM.commit();

}
