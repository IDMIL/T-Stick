//********************************************************************************//
//  Sopranino T-Stick 2G - Arduino Serial USB                                     //
//  First sketch: Joseph Malloch                                                  //
//  Input Devices and Music Interaction Laboratory                                //
//  Created: 2017/11/29 by Alex Nieva                                             //
//  Notes   : Adapted shiftIn() function by Carlyn Maw                            //
//            Adapted I2C code by Tom Igoe                                        //
//            Adapted LIS302DL code by Ben Gatti                                  //
//            EEPROM read/write functions adapted from code by Halley @arduino.cc //
//            Adapted to work with Arduino IDE 1.6.5 and T-Stick Sopranino 2G     //
//********************************************************************************//

// to do: jabCode, tapCode, read accels and piezo as fast as possible (1D of accels?)
// alex: deleted all the BYTE words in Serial.print() and changed Serial.print() to Serial.write()
// so that we send data as bytes and not ascii characters.
// The Sopranino hardware is different from the Soprano 2G T-Sticks, therefore this code
// will try to make it configurable similarly with the family of T-Sticks
// This should be compiled for the Arduino Pro Mini ATmega328P 3.3V 8MHz board.


// include Wire library to read and write I2C commands:
#include <Wire.h>
#include <EEPROM.h>
#include "eeprom_stuff.h"

#define DEBUG false
#define CALIB false

// defaults
unsigned int info[2] = {0, 0};    // serial number and firmware revision
boolean useI2C = false;
byte pressurePin = 6; //A more correct definition would be A6 but to keep consistency with old code on the 
byte piezoPin = 7;    //Arduino Mini 04 we use just numbers. 
boolean invertArray[5] = {0, 0, 0, 0, 0};
byte interval = 10;
byte jabInterval = 10;
unsigned int jabThresh = 512;
byte touchInterval = 15;
byte touchMask[2] = {255, 255};
unsigned int calibrationData[10] = {255, 767, 255, 767, 255, 767, 0, 1023, 0, 1023};


//ADXL345 accelerometer definitions
#define DEVICE (0x53) // Device address as specified in data sheet
#define ADXL345_MG2G_MULTIPLIER (0.0039)
#define SENSORS_GRAVITY_STANDARD          (SENSORS_GRAVITY_EARTH)
#define SENSORS_GRAVITY_EARTH             (9.80665F)              /**< Earth's gravity in m/s^2 */

byte buff[6];
char POWER_CTL = 0x2D;    //Power Control Register
char INT_ENABLE = 0x2E;   //Interrupt Enable Control
char DATA_FORMAT = 0x31;  //Data Format Control
char BW_RATE = 0x2C;      //Data Rate and Power Mode Control

char DATAX0 = 0x32;    //X-Axis Data 0
char DATAX1 = 0x33;    //X-Axis Data 1
char DATAY0 = 0x34;    //Y-Axis Data 0
char DATAY1 = 0x35;    //Y-Axis Data 1
char DATAZ0 = 0x36;    //Z-Axis Data 0
char DATAZ1 = 0x37;    //Z-Axis Data 1

//int x, y, z;
int outxyz[3];
double roll = 0.00, pitch = 0.00;       //Roll & Pitch are the angles which rotate by the axis X and y 

// Capsense definitions
// I2C adresses
#define I2C_ADDR0 0x00
#define I2C_ADDR1 0x02

// some CY8C201xx registers
#define INPUT_PORT0 0x00
#define INPUT_PORT1 0x01
#define CS_ENABLE0 0x06
#define CS_ENABLE1 0x07
#define I2C_DEV_LOCK 0x79
#define I2C_ADDR_DM 0x7C
#define COMMAND_REG 0xA0  //Sets the device in setup operation mode. In this mode, CapSense pin assignments can be changed as well
                          //as other parameters.

// Secret codes for locking/unlocking the I2C_DEV_LOCK register
byte I2CDL_KEY_UNLOCK[3] = {0x3C, 0xA5, 0x69};
byte I2CDL_KEY_LOCK[3] = {0x96, 0x5A, 0xC3};


//define pins for reset of capsense
int xres = 7;
int xres1 = 8;


//define values for slip coding
byte escapeChar = 101;
byte delimiterChar = 100;

//define codes for routing data
byte infoCode = 0;
byte touchCode = 1;
byte jabCode = 2;
byte tapCode = 3;
byte periodicCode = 4;
byte testCode = 5;

byte calibrate = 0;

int t;
int ledPin = 13;
int ledStatus = 0;
int ledTimer = 1000;
unsigned long then = 0;
unsigned long now = 0;
unsigned long started = 0;

byte touch[2] = {0, 0};
unsigned long touched = 0;

unsigned int acceleration[3] = {0, 0, 0};

void setup() {
  readSettings();
  
  //start serial
  Serial.begin(57600);

  //define pin modes
  pinMode(xres, OUTPUT);
  pinMode(xres1, OUTPUT); 

  //start I2C bus
  setupI2C();
  if (CALIB) calib();
}

void loop() {
  if(Serial.available()) {
    char message = Serial.read();
    switch (message) {
      case 's':          // start message, send serial#, revision first
        slipOut(infoCode);
        slipOutInt(info[0]);
        slipOutInt(info[1]);
        Serial.write(delimiterChar);
        started = millis();
        break;
      case 'i':   {       // send a lot of info
        slipOut(infoCode);
        slipOutInt(info[0]);
        slipOutInt(info[1]);
        slipOut(useI2C);
        slipOut(pressurePin);
        slipOut(piezoPin);
        byte temp = 0;
        for (t=0; t<5; t++) {
          temp = (temp << 0) + invertArray[t];
        }
        slipOut(temp);
        Serial.write(delimiterChar);}
        break;
      case 'x':          // stop message
        started = 0;
        break;
      case 'c':          // calibrate message
        while (Serial.available() < 1) {}
        switch (Serial.read()) {
          case 1:
            calibrationData[0] = 1023;
            calibrationData[1] = 0;
            calibrationData[2] = 1023;
            calibrationData[3] = 0;
            calibrationData[4] = 1023;
            calibrationData[5] = 0;
            calibrate = 1;
            break;
          case 2:
            calibrationData[6] = 1023;
            calibrationData[7] = 0;
            calibrate = 2;
            break;
          case 3:
            calibrationData[8] = 1023;
            calibrationData[9] = 0;
            calibrate = 3;
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
          case 'c':        //write calibration data
            while (Serial.available() < 20) {}
            for (t=0; t<20; t++) {
              calibrationData[t] = Serial.read()*256 + Serial.read();
            }
            break;
          case 'd':        // use I2C
            while (Serial.available() < 1) {}
            if (Serial.read()) {
              useI2C = 1;
              //setupI2C();
            } else
              useI2C = 0;
            break;
          case 'i':        // write info
            while (Serial.available() < 4) {}
            info[0] = Serial.read()*256 + Serial.read();
            info[1] = Serial.read()*256 + Serial.read();
            break;
          case 'I':        //write sensorInvert
            while (Serial.available() < 1) {}
            invertSensorData(Serial.read());
            break;
          case 'j':        //write jab interval
            while (Serial.available() < 1) {}
            jabInterval = Serial.read();
            break;
          case 'J':        //write jab threshold
            while (Serial.available() < 2) {}
            jabThresh = Serial.read()*256 + Serial.read();
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
            while (Serial.available() < 2) {}
            for (t=0; t<2; t++) {
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
        for (t=0; t<2; t++) {
          slipOut(touch[t]&touchMask[t]);
        }
        Serial.write(delimiterChar);
        touched = now;
      }
    }
    
/*    if ((now - jabbed) > jabInterval) { // throttles priority data, maybe unnecessary?
      if(readJab()) {
        slipOut(jabCode);
        for (t=0; t<3; t++) {
          slipOut(accelerationDelta[t]);
        }
        Serial.write(delimiterChar, BYTE);
        jabbed = now;
      }
    }*/
    
/*    if ((now - tapped) > tapInterval) { // throttles priority data, maybe unnecessary?
      if(readTap()) {
        slipOut(tapCode);
        slipOut(tap);
        Serial.write(delimiterChar, BYTE);
        tapped = now;
      }
    }*/
    
    //check elapsed time and output periodic data
    if ((now - then) > interval) {
      slipOut(periodicCode);
      readAccelerometer();
      for (t=0; t<3; t++) {        
        slipOutInt(constrain(map(acceleration[t], calibrationData[t * 2 + invertArray[t]], calibrationData[t * 2 + 1 - invertArray[t]], 255, 767), 0, 1023));
      }

      unsigned int avtemp = 0; //Averaging 5 times for paper-based sensor.
      for (int i=0; i<5; i++){
        avtemp += readPressure();  
      }
      avtemp = avtemp/5;

      slipOutInt(constrain(map(avtemp, calibrationData[6 + invertArray[3]], calibrationData[7 - invertArray[3]], 0, 1023), 0, 1023));
      slipOutInt(constrain(map(readPiezo(), calibrationData[8 + invertArray[4]], calibrationData[9 - invertArray[4]], 0, 1023), 0, 1023));
      Serial.write(delimiterChar);
//      ledBlink(); // No LED implemented in this prototype. There is one onboard that can be barely seen through the acrylic helps
                    // monitor that the T-Stick is in stand-by mode.
      then = now;
    }
    else if (then > now)
      then = 0;
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

byte readTouch() {
  boolean changed = 0;

  // get touch data from I2C_ADDR0
  // request Register 00h: INPUT_PORT0
  Wire.beginTransmission(I2C_ADDR0);
  Wire.write(INPUT_PORT0);
  Wire.endTransmission();
  
  Wire.requestFrom(I2C_ADDR0, 1);
  while (Wire.available()) {
  touch[0] = Wire.read() << 4;
  }
  // request Register 01h: INPUT_PORT1
  Wire.beginTransmission(I2C_ADDR0);
  Wire.write(INPUT_PORT1);
  Wire.endTransmission();
  
  Wire.requestFrom(I2C_ADDR0, 1);
  while (Wire.available()) {
  touch[0] |= Wire.read();
  }
  
  // get touch data from I2C_ADDR1
  // request Register 00h: INPUT_PORT0
  Wire.beginTransmission(I2C_ADDR1);
  Wire.write(INPUT_PORT0);
  Wire.endTransmission();
  
  Wire.requestFrom(I2C_ADDR1, 1);
  while (Wire.available()) {
  touch[1] = Wire.read() << 4;
  }
  // request Register 01h: INPUT_PORT1
  Wire.beginTransmission(I2C_ADDR1);
  Wire.write(INPUT_PORT1);
  Wire.endTransmission();
  
  Wire.requestFrom(I2C_ADDR1, 1);
  while (Wire.available()) {
  touch[1] |= Wire.read();
  }
  changed = 1;

  byte touch_temp[2] = {0,0};
  for (int i=0; i<8; i++){ //Re-mapping the Capsense pins instead of doing it in the Max Patch.
    byte mult = 1<<i;
    byte temp0 = ((touch[0] & mult)>>i)%2;
    byte temp1 = ((touch[1] & mult)>>i)%2;
    switch (i) {
      case 0:
        touch_temp[0] |= temp0 << 7;
        touch_temp[1] |= temp1 << 7;
        break;
      case 1:
        touch_temp[0] |= temp0 << 4;
        touch_temp[1] |= temp1 << 4;
        break;
      case 2:
        touch_temp[0] |= temp0 << 5;
        touch_temp[1] |= temp1 << 5;
        break;
      case 3:
        touch_temp[0] |= temp0 << 2;
        touch_temp[1] |= temp1 << 2;
        break;
      case 4:
        touch_temp[0] |= temp0 << 1;
        touch_temp[1] |= temp1 << 1;
        break;
      case 5:
        touch_temp[0] |= temp0 << 6;
        touch_temp[1] |= temp1 << 6;
        break;
      case 6:
        touch_temp[0] |= temp0 << 3;
        touch_temp[1] |= temp1 << 3;
        break;
      case 7:
        touch_temp[0] |= temp0 << 0;
        touch_temp[1] |= temp1 << 0;
        break;
    }
  }

  touch[0] = touch_temp[0];
  touch[1] = touch_temp[1];
  return changed;
}

void slipOut(byte output) {
    if ((output==escapeChar)||(output==delimiterChar)) Serial.write(escapeChar);
    Serial.write(output);
}

void slipOutInt( int output ){
  slipOut( byte(output/256));
  slipOut( byte(output%256));  
}

unsigned int readPressure() {
  unsigned int temp = analogRead(pressurePin);
  if (calibrate == 2) {
    calibrationData[6] = constrain(min(calibrationData[6], temp), 0, 1023);
    calibrationData[7] = constrain(max(calibrationData[7], temp), 0, 1023);
  }
  return temp;
}

unsigned int readPiezo() {
  unsigned int temp = analogRead(piezoPin);
  if (calibrate == 3) {
    calibrationData[8] = constrain(min(calibrationData[8], temp), 0, 1023);
    calibrationData[9] = constrain(max(calibrationData[9], temp), 0, 1023);
  }
  return temp;
}

void invertSensorData(byte sensorInvert) 
{
  if (sensorInvert > 0) {
    unsigned int temp;
    for (t=0; t<5; t++) {
      invertArray[t] = (sensorInvert >> t) & 1;
    }
  } else
    boolean invertArray[5] = {0, 0, 0, 0, 0};
}

void writeTo(byte address, byte val) 
{
  Wire.beginTransmission(DEVICE); // start transmission to device
  Wire.write(address); // send register address
  Wire.write(val); // send value to write
  Wire.endTransmission(); // end transmission
}


// Reads num bytes starting from address register on device in to _buff array
void readFrom(byte address, int num, byte buff[]) 
{
  Wire.beginTransmission(DEVICE); // start transmission to device
  Wire.write(address); // sends address to read from
  Wire.endTransmission(); // end transmission
  Wire.beginTransmission(DEVICE); // start transmission to device
  Wire.requestFrom(DEVICE, num); // request 6 bytes from device

  int i = 0;
  while(Wire.available()) // device may send less than requested (abnormal)
  {
    buff[i] = Wire.read(); // receive a byte
    i++;
  }
  Wire.endTransmission(); // end transmission
}

boolean readSettings() {
  byte written;
  int n = EEPROM_readAnything(0, written);    //checks if any data written before overwriting defaults
  
  if (written == 100) {
    n = EEPROM_readAnything(n, info);
    n = EEPROM_readAnything(n, useI2C);
    n = EEPROM_readAnything(n, pressurePin);
    n = EEPROM_readAnything(n, piezoPin);
    n = EEPROM_readAnything(n, invertArray);
    n = EEPROM_readAnything(n, interval);
    n = EEPROM_readAnything(n, jabInterval);
    n = EEPROM_readAnything(n, jabThresh);
    n = EEPROM_readAnything(n, touchInterval);
    n = EEPROM_readAnything(n, touchMask);
    n = EEPROM_readAnything(n, calibrationData);
  }
}

boolean writeSettings() {
  byte written = 100;
  int n = EEPROM_writeAnything(0, written);
  
  n = EEPROM_writeAnything(n, info);
  n = EEPROM_writeAnything(n, useI2C);
  n = EEPROM_writeAnything(n, pressurePin);
  n = EEPROM_writeAnything(n, piezoPin);
  n = EEPROM_writeAnything(n, invertArray);
  n = EEPROM_writeAnything(n, interval);
  n = EEPROM_writeAnything(n, jabInterval);
  n = EEPROM_writeAnything(n, jabThresh);
  n = EEPROM_writeAnything(n, touchInterval);
  n = EEPROM_writeAnything(n, touchMask);
  n = EEPROM_writeAnything(n, calibrationData);
}
