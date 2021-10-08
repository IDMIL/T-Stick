//********************************************************************************//
//  Soprano T-Stick 2G-IMU (LSM9DS0) - Arduino Serial USB                         //
//  Joseph Malloch                                                                //
//  Modified by Alex Nieva                                                        //
//  Input Devices and Music Interaction Laboratory                                //
//  Modified : 2017/04/14                                                         //
//  Notes   : Adapted shiftIn() function by Carlyn Maw                            //
//            Adapted I2C code by Tom Igoe                                        //
//                                                                                //
//            Using LSM9DS0 IMU. Sparkfun library                                 //
//            EEPROM read/write functions adapted from code by Halley @arduino.cc //
//********************************************************************************//


// include Wire library to read and write I2C commands:
#include <SPI.h>
#include <Wire.h>
#include <SFE_LSM9DS0.h>
#include <EEPROM.h>
#include "eeprom_stuff.h"

#define DEBUG false

///////////////////////
// I2C Setup //
///////////////////////
// Comment out this section if you're using SPI
// SDO_XM and SDO_G are both grounded, so our addresses are:
#define LSM9DS0_XM  0x1D // Would be 0x1E if SDO_XM is LOW
#define LSM9DS0_G   0x6B // Would be 0x6A if SDO_G is LOW
// Create an instance of the LSM9DS0 library called `dof` the
// parameters for this constructor are:
// [SPI or I2C Mode declaration],[gyro I2C address],[xm I2C add.]
LSM9DS0 dof(MODE_I2C, LSM9DS0_G, LSM9DS0_XM);

// defaults
byte info[4] = {0, 0, 0, 0};    // serial number and firmware revision
byte pressurePin = 3;
byte piezoPin = 6;
byte interval = 10;
byte touchInterval = 1;
byte magInterval = 20;
byte touchMask[6] = {255, 255, 255, 255, 255, 255};
unsigned int pressureCalibration[] = {0, 1023};
unsigned int piezoCalibration[] = {0, 1023};
int invert = 0;
boolean accelerometerInvert[] = {0, 0, 0};
boolean gyroscopeInvert[] = {0, 0, 0};
boolean magnetometerInvert[] = {0, 0, 0};
boolean pressureInvert = 0;
boolean piezoInvert = 0;
//This added for LSM9DS0
float abias[3] = {0, 0, 0}, gbias[3] = {0, 0, 0};

//define pins for shift registers
int clockPin = 7;
int latchPin = 8;
int dataPin = 9;

//define values for slip coding
byte escapeChar = 101;
byte delimiterChar = 100;

//define codes for routing data
byte infoCode = 0;
byte touchCode = 1;
byte periodicCode = 2;
byte magCode = 3;

byte calibrate = 0;

int t;
int ledPin = 13;
int ledStatus = 0;
int ledTimer = 1000;
unsigned long then = 0;
unsigned long now = 0;
unsigned long started = 0;
unsigned long magTime = 0;

byte touch[6] = {0, 0, 0, 0, 0, 0};
unsigned long touched = 0;


void setup() {
  readSettings();
  
  //start serial
  Serial.begin(115200);
  
  
  // start I2C communication
  //Wire.begin();
  uint16_t status = dof.begin();
  if (DEBUG){
    Serial.print("LSM9DS0 WHO_AM_I's returned: 0x");
    Serial.println(status, HEX);
    Serial.println("Should be 0x49D4");
    Serial.println();
  }    

  // Set data output ranges; choose lowest ranges for maximum resolution
  // Accelerometer scale can be: A_SCALE_2G, A_SCALE_4G, A_SCALE_6G, A_SCALE_8G, or A_SCALE_16G   
  dof.setAccelScale(dof.A_SCALE_4G);
  // Gyro scale can be:  G_SCALE__245, G_SCALE__500, or G_SCALE__2000DPS
  dof.setGyroScale(dof.G_SCALE_245DPS);
  // Magnetometer scale can be: M_SCALE_2GS, M_SCALE_4GS, M_SCALE_8GS, M_SCALE_12GS   
  dof.setMagScale(dof.M_SCALE_2GS);
  
  // Set output data rates  
  // Accelerometer output data rate (ODR) can be: A_ODR_3125 (3.225 Hz), A_ODR_625 (6.25 Hz), A_ODR_125 (12.5 Hz), A_ODR_25, A_ODR_50, 
  //                                              A_ODR_100,  A_ODR_200, A_ODR_400, A_ODR_800, A_ODR_1600 (1600 Hz)
  dof.setAccelODR(dof.A_ODR_200); // Set accelerometer update rate at 200 Hz
  
  // Accelerometer anti-aliasing filter rate can be 50, 194, 362, or 763 Hz
  // Anti-aliasing acts like a low-pass filter allowing oversampling of accelerometer and rejection of high-frequency spurious noise.
  // Strategy here is to effectively oversample accelerometer at 100 Hz and use a 50 Hz anti-aliasing (low-pass) filter frequency
  // to get a smooth ~150 Hz filter update rate
  dof.setAccelABW(dof.A_ABW_50); // Choose lowest filter setting for low noise
  
  // Gyro output data rates can be: 95 Hz (bandwidth 12.5 or 25 Hz), 190 Hz (bandwidth 12.5, 25, 50, or 70 Hz)
  //                                 380 Hz (bandwidth 20, 25, 50, 100 Hz), or 760 Hz (bandwidth 30, 35, 50, 100 Hz)
  dof.setGyroODR(dof.G_ODR_190_BW_125);  // Set gyro update rate to 190 Hz with the smallest bandwidth for low noise
  
  // Magnetometer output data rate can be: 3.125 (ODR_3125), 6.25 (ODR_625), 12.5 (ODR_125), 25, 50, or 100 Hz
  dof.setMagODR(dof.M_ODR_125); // Set magnetometer to update every 80 ms
  
  // Use the FIFO mode to average accelerometer and gyro readings to calculate the biases, which can then be removed from
  // all subsequent measurements.
  dof.calLSM9DS0(gbias, abias);
  delay(1000);

  //define pin modes
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT); 
  pinMode(dataPin, INPUT);
  pinMode(ledPin, OUTPUT);
  
  delay(5);
  
}

void loop() {
  if(Serial.available()) {
    char message = Serial.read();
  
    switch (message) {       
      case 's':          // start message, send serial#, revision first
        slipOut(infoCode);
        for (t=0; t<4; t++)
          slipOut(info[t]);
        Serial.write(delimiterChar);
        started = millis();
        break;
      case 'x':          // stop message
        started = 0;
        break;
      case 'c':          // calibrate message
        while (Serial.available() < 1) {}
        //message = Serial.read();
        switch (Serial.read()) {
          case 1:
            // pressure sensor calibration
            pressureCalibration[0] = 1023;
            pressureCalibration[1] = 0;
            calibrate = 3;
            break;
          case 2:
            piezoCalibration[0] = 1023;
            piezoCalibration[1] = 0;
            calibrate = 4;
            break;
          default:
            calibrate = 0;
            break;
        }
        break;
      case 'w':          // write settings
        while (Serial.available() < 1) {}
        message = Serial.read();
        switch (message) {
          case 'i':        // write info
            while (Serial.available() < 4) {}
            for (t=0; t<4; t++)
              info[t] = Serial.read();
            break;
          case 'I':        //write sensorInvert
            while (Serial.available() < 2) {}
            invert = Serial.read() << 8;
            invert |= Serial.read();
            invertSensorData(invert);
            break;
          case 'p':        //write period
            while (Serial.available() < 1) {}
            interval = Serial.read();
            break;
          case 'P':        //write sensor pins
            while (Serial.available() < 2) {}
            pressurePin = Serial.read();
            piezoPin = Serial.read();
            break;
          case 't':        //write touch interval
            while (Serial.available() < 1) {}
            touchInterval = Serial.read();
            break;
          case 'T':        //write touch mask
            while (Serial.available() < 12) {}
            for (t=0; t<6; t++) {
              touchMask[t] = Serial.read();
            }
            break;
          case 'w':
            writeSettings();
            break;
          default:
          break;
        } 
    }
  }
  
  now = millis();
  
  if (now < started + 2000) {  // needs a ping/keep-alive every 2 seconds or less or will timesout
    
    //check priority data: Touch data only for now
    if ((now - touched) > touchInterval) { // throttles priority data, maybe unnecessary?
      if(readTouch()) {
        slipOut(touchCode);
        for (t=0; t<6; t++) {
          slipOut(touch[t]);
        }
        Serial.write(delimiterChar);
        touched = now;
      }
    }
    
    //check elapsed time and output periodic data
    if ((now - then) > interval) {
      slipOut(periodicCode);
      
      readAccelerometer();
      readGyroscope();

      slipOutInt(constrain(map(readPressure(), pressureCalibration[pressureInvert], pressureCalibration[1 - pressureInvert], 0, 1023), 0, 1023));
      slipOutInt(constrain(map(readPiezo(), piezoCalibration[piezoInvert], piezoCalibration[1 - piezoInvert], 0, 1023), 0, 1023));
      Serial.write(delimiterChar);
      ledBlink();
      then = now;
    }
    else if (then > now)
      then = 0;

    if ((now - magTime) > magInterval) {
      slipOut(magCode);
      readMagnetometer();
      Serial.write(delimiterChar);
      magTime = now;
    }
    else if (magTime > now)
      magTime = 0;
  }
  else {
    now = millis();
    if ((now - then) > ledTimer) {
      ledBlink();
      then = now;
    }
    else if (then > now)
      then = 0;
  }
}

void ledBlink() {
  ledStatus = (ledStatus + 1) % 2;
  digitalWrite(ledPin, ledStatus);
}

boolean readTouch() {
  boolean changed = 0;
  digitalWrite(latchPin, 0);  //set latch pin to 0 to transmit data serially  
  for (t=0; t<6; t++) {
    byte temp = shiftIn(dataPin, clockPin) & touchMask[t];  //Read digital data
    if (temp != touch[t]) {
      changed = 1;
      touch[t] = temp;
    }
  }
  digitalWrite(latchPin, 1);  //set latch pin to 1 to collect parallel data
  return changed;
}

byte shiftIn(int myDataPin, int myClockPin) { 
  int i;
  int temp = 0;
  byte myDataIn = 0;
  for (i=7; i>=0; i--)
  {
    digitalWrite(myClockPin, 0);
    temp = digitalRead(myDataPin);
    if (temp) {
      myDataIn = myDataIn | (1 << i);
    }
    digitalWrite(myClockPin, 1);
  }
  return myDataIn;
}

void slipOut(byte output) {
    if ((output==escapeChar)||(output==delimiterChar)) Serial.write(escapeChar);
    Serial.write(output);
}

void slipOutInt( int output ){
  slipOut( byte(output >> 8));
  slipOut( byte(output & 0xFF));  
}

void readAccelerometer() {
//  accel.readAccel(acceleration);
  dof.readAccel();
  slipOutInt(dof.ay); // This effectively rotates the axis making X along the T-Stick as defined in Alex Nieva's Thesis
  slipOutInt(-dof.ax); // This rotates Y so that positive points towars the user with the capacitive keys pointing upwards
  slipOutInt(dof.az);
}

void readGyroscope() {
  dof.readGyro();
  slipOutInt(dof.gy); // Rotation same as accelerometer.
  slipOutInt(-dof.gx);
  slipOutInt(dof.gz);
}

void readMagnetometer() {
  dof.readMag();
  slipOutInt(dof.my); // Rotation same as accelerometer.
  slipOutInt(-dof.mx);
  slipOutInt(dof.mz); 
}

unsigned int readPressure() {
  unsigned int temp = analogRead(pressurePin);
  if (calibrate == 3) {
    pressureCalibration[0] = constrain(min(pressureCalibration[0], temp), 0, 1023);
    pressureCalibration[1] = constrain(max(pressureCalibration[1], temp), 0, 1023);
  }
  return temp;
}

unsigned int readPiezo() {
  unsigned int temp = analogRead(piezoPin);
  if (calibrate == 4) {
    piezoCalibration[0] = constrain(min(piezoCalibration[0], temp), 0, 1023);
    piezoCalibration[1] = constrain(max(piezoCalibration[1], temp), 0, 1023);
  }
  return temp;
}

void invertSensorData(int sensorInvert) {
  if (sensorInvert <= 0) {
    invert = 0;
  }
  else {
    int i = 0;
    for (t=0; t<3; t++) {
      accelerometerInvert[t] = (sensorInvert >> i++) & 1;
    }
    for (t=0; t<3; t++) {
      gyroscopeInvert[t] = (sensorInvert >> i++) & 1;
    }
    for (t=0; t<3; t++) {
      magnetometerInvert[t] = (sensorInvert >> i++) & 1;
    }
    pressureInvert = (sensorInvert >> i++) & 1;
    piezoInvert = (sensorInvert >> i++) & 1;
  }
}

boolean readSettings() {
  byte written;
  int n = EEPROM_readAnything(0, written);    //checks if any data written before overwriting defaults
  
  if (written == 100) {
    n = EEPROM_readAnything(n, info);
//    n = EEPROM_readAnything(n, accelerometerOrder);
//    n = EEPROM_readAnything(n, gyroscopeOrder);
//    n = EEPROM_readAnything(n, magnetometerOrder);
    n = EEPROM_readAnything(n, pressurePin);
    n = EEPROM_readAnything(n, piezoPin);
    n = EEPROM_readAnything(n, accelerometerInvert);
    n = EEPROM_readAnything(n, gyroscopeInvert);
    n = EEPROM_readAnything(n, magnetometerInvert);
    n = EEPROM_readAnything(n, pressureInvert);
    n = EEPROM_readAnything(n, piezoInvert);
//    n = EEPROM_readAnything(n, accelerometerCalibration);
//    n = EEPROM_readAnything(n, gyroscopeCalibration);
//    n = EEPROM_readAnything(n, magnetometerCalibration);
    n = EEPROM_readAnything(n, interval);
    n = EEPROM_readAnything(n, touchInterval);
    n = EEPROM_readAnything(n, touchMask);
  }
}

boolean writeSettings() {
  byte written = 100;
  int n = EEPROM_writeAnything(0, written);
  
  n = EEPROM_writeAnything(n, info);
//  n = EEPROM_writeAnything(n, accelerometerOrder);
//  n = EEPROM_writeAnything(n, gyroscopeOrder);
//  n = EEPROM_writeAnything(n, magnetometerOrder);
  n = EEPROM_writeAnything(n, pressurePin);
  n = EEPROM_writeAnything(n, piezoPin);
  n = EEPROM_writeAnything(n, accelerometerInvert);
  n = EEPROM_writeAnything(n, gyroscopeInvert);
  n = EEPROM_writeAnything(n, magnetometerInvert);
  n = EEPROM_writeAnything(n, pressureInvert);
  n = EEPROM_writeAnything(n, piezoInvert);
//  n = EEPROM_writeAnything(n, accelerometerCalibration);
//  n = EEPROM_writeAnything(n, gyroscopeCalibration);
//  n = EEPROM_writeAnything(n, magnetometerCalibration);
  n = EEPROM_writeAnything(n, interval);
  n = EEPROM_writeAnything(n, touchInterval);
  n = EEPROM_writeAnything(n, touchMask);
}
