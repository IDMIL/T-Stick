//********************************************************************************//
//  Soprano T-Stick 2GX - Arduino Serial USB                                       //
//  Joseph Malloch 14/01/2010                                                     //
//  Input Devices and Music Interaction Laboratory                                //
//  Version : 1.2.1 (prioritizing)                                                //
//  Notes   : Adapted shiftIn() function by Carlyn Maw                            //
//            Adapted I2C code by Tom Igoe                                        //
//            Adapted LIS302DL code by Ben Gatti                                  //
//            EEPROM read/write functions adapted from code by Halley @arduino.cc //
//********************************************************************************//

//  to do:
//    jabCode
//    tapCode
//    read accels and piezo as fast as possible (1D of accels?)
//    read and report current settings

// include Wire library to read and write I2C commands:
#include <Wire.h>
#include <EEPROM.h>

template <class T> int EEPROM_writeAnything(int ee, const T& value)
{
    const byte* p = (const byte*)(const void*)&value;
    int i;
    for (i = 0; i < sizeof(value); i++)
	  EEPROM.write(ee++, *p++);
    return ee;
}

template <class T> int EEPROM_readAnything(int ee, T& value)
{
    byte* p = (byte*)(void*)&value;
    int i;
    for (i = 0; i < sizeof(value); i++)
	  *p++ = EEPROM.read(ee++);
    return ee;
}

// defaults
unsigned int info[2] = {0, 0};    // serial number and firmware revision
boolean useI2C = true;
byte accelxPin = 0;
byte accelyPin = 1;
byte accelzPin = 2;
byte pressurePin = 0;
byte piezoPin = 1;
byte airPressurePin = 4;
byte rangePin = 5;
byte lightPin[2] = {2, 3};
boolean invertArray[8] = {0, 0, 0, 0, 0, 0, 0, 0};
byte interval = 10;
byte jabInterval = 10;
unsigned int jabThresh = 512;
byte touchInterval = 1;
byte touchMask[6] = {255, 255, 255, 255, 255, 255};
unsigned int calibrationData[18] = {255, 767, 255, 767, 255, 767, 0, 1023, 0, 1023, 0, 1023, 0, 1023, 0, 1023, 0, 1023};


//LIS302DL accelerometer addresses
#define accel1Address 0x1C
#define accelResultX 0x29
#define accelResultY 0x2B
#define accelResultZ 0x2D

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
byte jabCode = 2;
byte tapCode = 3;
byte periodicCode = 4;

byte calibrate = 0;

int t;
int ledPin = 13;
int ledStatus = 0;
int ledTimer = 1000;
unsigned long then = 0;
unsigned long now = 0;
unsigned long started = 0;

byte touch[6] = {0, 0, 0, 0, 0, 0};
unsigned long touched = 0;

unsigned int acceleration[3] = {0, 0, 0};
//byte accelerationDelta[3] = {0, 0, 0};
//unsigned long jabbed = 0;

void setup() {
  
  readSettings();
  
  //start serial
  Serial.begin(115200);

  //define pin modes
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT); 
  pinMode(dataPin, INPUT);
  pinMode(ledPin, OUTPUT);
  
  if ( useI2C ){
    setupI2C();      
  }
}

void loop() {
  if(Serial.available()) {
    char message = Serial.read();
  
    switch (message) {
      case 's':          // start message, send serial#, revision first
        slipOut(infoCode);
        slipOutInt(info[0]);
        slipOutInt(info[1]);
        Serial.print(delimiterChar, BYTE);
        started = millis();
        break;
      case 'x':          // stop message
        started = 0;
        break;
      case 'c':          // calibrate message
        while (Serial.available() < 1) {}
        //message = Serial.read();
        switch (Serial.read()) {
          case 1: //calibrate accelerometers
            calibrationData[0] = 1023;
            calibrationData[1] = 0;
            calibrationData[2] = 1023;
            calibrationData[3] = 0;
            calibrationData[4] = 1023;
            calibrationData[5] = 0;
            calibrate = 1;
            break;
          case 2: //calibrate pressure sensor
            calibrationData[6] = 1023;
            calibrationData[7] = 0;
            calibrate = 2;
            break;
          case 3: //calibrate piezo sensor
            calibrationData[8] = 1023;
            calibrationData[9] = 0;
            calibrate = 3;
            break;
          case 4: //calibrate air pressure sensor
            calibrationData[10] = 1023;
            calibrationData[11] = 0;
            calibrate = 4;
            break;
          case 5: //calibrate IR range sensor
            calibrationData[12] = 1023;
            calibrationData[13] = 0;
            calibrate = 5;
            break;
          case 6: //calibrate LDR sensors
            calibrationData[14] = 1023;
            calibrationData[15] = 0;
            calibrationData[16] = 1023;
            calibrationData[17] = 0;
            calibrate = 6;
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
            while (Serial.available() < 18) {}
            for (t=0; t<18; t++) {
              calibrationData[t] = Serial.read()*256 + Serial.read();
            }
            break;
          case 'd':        // use I2C
            while (Serial.available() < 1) {}
            if (Serial.read()) {
              useI2C = 1;
              setupI2C();
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
            while (Serial.available() < 5) {}
            accelxPin = Serial.read();
            accelyPin = Serial.read();
            accelzPin = Serial.read();
            pressurePin = Serial.read();
            piezoPin = Serial.read();
            airPressurePin = Serial.read();
            rangePin = Serial.read();
            lightPin[0] = Serial.read();
            lightPin[1] = Serial.read();
            break;
          case 't':        //write touch interval
            while (Serial.available() < 1) {}
            touchInterval = Serial.read();
            break;
          case 'T':        //write touch mask
            while (Serial.available() < 6) {}
            for (t=0; t<6; t++) {
              touchMask[t] = Serial.read();
            }
            break;
          case 'w':
            writeSettings();
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
        Serial.print(delimiterChar, BYTE);
        touched = now;
      }
    }
    
/*    if ((now - jabbed) > jabInterval) { // throttles priority data, maybe unnecessary?
      if(readJab()) {
        slipOut(jabCode);
        for (t=0; t<3; t++) {
          slipOut(accelerationDelta[t]);
        }
        Serial.print(delimiterChar, BYTE);
        jabbed = now;
      }
    }*/
    
/*    if ((now - tapped) > tapInterval) { // throttles priority data, maybe unnecessary?
      if(readTap()) {
        slipOut(tapCode);
        slipOut(tap);
        Serial.print(delimiterChar, BYTE);
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
      slipOutInt(constrain(map(readPressure(), calibrationData[6 + invertArray[3]], calibrationData[7 - invertArray[3]], 0, 1023), 0, 1023));
      slipOutInt(constrain(map(readPiezo(), calibrationData[8 + invertArray[4]], calibrationData[9 - invertArray[4]], 0, 1023), 0, 1023));
      slipOutInt(constrain(map(readAirPressure(), calibrationData[10 + invertArray[5]], calibrationData[11 - invertArray[5]], 0, 1023), 0, 1023));
      slipOutInt(constrain(map(readRange(), calibrationData[12 + invertArray[6]], calibrationData[13 - invertArray[6]], 0, 1023), 0, 1023));
      slipOutInt(constrain(map(readLight(0), calibrationData[14 + invertArray[7]], calibrationData[15 - invertArray[7]], 0, 1023), 0, 1023));
      slipOutInt(constrain(map(readLight(1), calibrationData[16 + invertArray[7]], calibrationData[17 - invertArray[7]], 0, 1023), 0, 1023));
      Serial.print(delimiterChar, BYTE);
      ledBlink();
      then = now;
    }
    else if (then > now)
      then = 0;
  }
  else {
    if ((now - then) > ledTimer) {
      ledBlink();
      then = now;
    }
    else if (then > now)
      then = 0;
    delay(100);
  }
}

void ledBlink() {
  ledStatus = (ledStatus + 1) % 2;
  digitalWrite(ledPin, ledStatus);
}

boolean readTouch() {
  boolean changed = 0;
  digitalWrite(latchPin,0);  //set latch pin to 0 to transmit data serially  
  for (t=0; t<6; t++) {
    byte temp = shiftIn(dataPin, clockPin) & touchMask[t];  //Read digital data
    if (temp != touch[t]) {
      changed = 1;
      touch[t] = temp;
    }
  }
  digitalWrite(latchPin,1);  //set latch pin to 1 to collect parallel data
  return changed;
}

boolean readJab() {
  boolean changed = 0;
  byte accelerationOld[3];
  for (t=0; t<3; t++) {
    accelerationOld[t] = acceleration[t];
  }
  readAccelerometer();
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
    if ((output==escapeChar)||(output==delimiterChar)) Serial.print(escapeChar, BYTE);
    Serial.print(output, BYTE);
}

void slipOutInt( int output ){
  slipOut( byte(output/256));
  slipOut( byte(output%256));  
}

void readAccelerometer() {
  if (useI2C) {
    int address = accel1Address;
    
    Wire.beginTransmission(address);
    Wire.send(accelResultX);                   //set x register
    Wire.endTransmission();
    Wire.requestFrom(address, 1);            //retrieve x value
    acceleration[accelxPin % 3] = (Wire.receive() + 128) % 256;
    
    Wire.beginTransmission(address);
    Wire.send(accelResultY);                   //set y register
    Wire.endTransmission();
    Wire.requestFrom(address, 1);            //retrieve y value
    acceleration[accelyPin % 3] = (Wire.receive() + 128) % 256;
  
    Wire.beginTransmission(address);
    Wire.send(accelResultZ);                   //set z register
    Wire.endTransmission();
    Wire.requestFrom(address, 1);            //retrieve z value
    acceleration[accelzPin % 3] = (Wire.receive() + 128) % 256;
  } else {
    acceleration[0] = analogRead(accelxPin);
    acceleration[1] = analogRead(accelyPin);
    acceleration[2] = analogRead(accelzPin);
  }
  
  if (calibrate == 1) {
    calibrationData[0] = constrain(min(calibrationData[0], acceleration[0]), 0, 1023);
    calibrationData[1] = constrain(max(calibrationData[1], acceleration[0]), 0, 1023);
    calibrationData[2] = constrain(min(calibrationData[2], acceleration[1]), 0, 1023);
    calibrationData[3] = constrain(max(calibrationData[3], acceleration[1]), 0, 1023);
    calibrationData[4] = constrain(min(calibrationData[4], acceleration[2]), 0, 1023);
    calibrationData[5] = constrain(max(calibrationData[5], acceleration[2]), 0, 1023);
  }
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

unsigned int readAirPressure() {
  unsigned int temp = analogRead(airPressurePin);
  if (calibrate == 4) {
    calibrationData[10] = constrain(min(calibrationData[10], temp), 0, 1023);
    calibrationData[11] = constrain(max(calibrationData[11], temp), 0, 1023);
  }
  return temp;
}

unsigned int readRange() {
  unsigned int temp = analogRead(rangePin);
  if (calibrate == 5) {
    calibrationData[12] = constrain(min(calibrationData[12], temp), 0, 1023);
    calibrationData[13] = constrain(max(calibrationData[13], temp), 0, 1023);
  }
  return temp;
}

unsigned int readLight(byte number) {
  unsigned int temp = analogRead(lightPin[number]);
  if (calibrate == 6) {
    calibrationData[14 + number*2] = constrain(min(calibrationData[14 + number*2], temp), 0, 1023);
    calibrationData[15 + number*2] = constrain(max(calibrationData[15 + number*2], temp), 0, 1023);
  }
  return temp;
}

void setupI2C(){
   //start I2C bus
  Wire.begin();
  
  //LIS302DL setup
  Wire.beginTransmission(accel1Address);
  Wire.send(0x21); // CTRL_REG2 (21h)
  Wire.send(B01000000);
  Wire.endTransmission();
  
  	//SPI 4/3 wire
  	//1=ReBoot - reset chip defaults
  	//n/a
  	//filter off/on
  	//filter for freefall 2
  	//filter for freefall 1
  	//filter freq MSB
  	//filter freq LSB - Hipass filter (at 400hz) 00=8hz, 01=4hz, 10=2hz, 11=1hz (lower by 4x if sample rate is 100hz)   

  Wire.beginTransmission(accel1Address);
  Wire.send(0x20); // CTRL_REG1 (20h)
  Wire.send(B01000111);
  Wire.endTransmission();
  
  	//sample rate 100/400hz
  	//power off/on
  	//2g/8g
  	//self test
  	//self test
  	//z enable
  	//y enable
  	//x enable 
}

/*void calibrate(byte stage) {
  
  switch (stage) {
    case 0:
      // read accelerometer and save min and max for calibration
      readAccelerometer();
      calibrationData[0] = constrain(min(calibrationData[0], acceleration[0]), 0, 1023);
      calibrationData[1] = constrain(max(calibrationData[1], acceleration[0]), 0, 1023);
      calibrationData[2] = constrain(min(calibrationData[2], acceleration[1]), 0, 1023);
      calibrationData[3] = constrain(max(calibrationData[3], acceleration[1]), 0, 1023);
      calibrationData[4] = constrain(min(calibrationData[4], acceleration[2]), 0, 1023);
      calibrationData[5] = constrain(max(calibrationData[5], acceleration[2]), 0, 1023);
      break;
    case 1:
      // read pressure and save min and max for calibration
      calibrationData[6] = constrain(min(calibrationData[6], analogRead(pressurePin)), 0, 1023);
      calibrationData[7] = constrain(max(calibrationData[7], analogRead(pressurePin)), 0, 1023);
      break;
    case 2:
      // read piezo and save min and max for calibration
      calibrationData[8] = constrain(min(calibrationData[8], analogRead(piezoPin)), 0, 1023);
      calibrationData[9] = constrain(max(calibrationData[9], analogRead(piezoPin)), 0, 1023);
      break;
  }
}*/

void invertSensorData(byte sensorInvert) {
  if (sensorInvert > 0) {
    unsigned int temp;
    for (t=0; t<8; t++) {
      invertArray[t] = (sensorInvert >> t) & 1;
    }
  } else
    boolean invertArray[8] = {0, 0, 0, 0, 0, 0, 0, 0};
}

boolean readSettings() {
  byte written;
  int n = EEPROM_readAnything(0, written);    //checks if any data written before overwriting defaults
  
  if (written == 100) {
    n = EEPROM_readAnything(n, info);
    n = EEPROM_readAnything(n, useI2C);
    n = EEPROM_readAnything(n, accelxPin);
    n = EEPROM_readAnything(n, accelyPin);
    n = EEPROM_readAnything(n, accelzPin);
    n = EEPROM_readAnything(n, pressurePin);
    n = EEPROM_readAnything(n, piezoPin);
    n = EEPROM_readAnything(n, airPressurePin);
    n = EEPROM_readAnything(n, rangePin);
    n = EEPROM_readAnything(n, lightPin);
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
  n = EEPROM_writeAnything(n, accelxPin);
  n = EEPROM_writeAnything(n, accelyPin);
  n = EEPROM_writeAnything(n, accelzPin);
  n = EEPROM_writeAnything(n, pressurePin);
  n = EEPROM_writeAnything(n, piezoPin);
  n = EEPROM_writeAnything(n, airPressurePin);
  n = EEPROM_writeAnything(n, rangePin);
  n = EEPROM_writeAnything(n, lightPin);
  n = EEPROM_writeAnything(n, invertArray);
  n = EEPROM_writeAnything(n, interval);
  n = EEPROM_writeAnything(n, jabInterval);
  n = EEPROM_writeAnything(n, jabThresh);
  n = EEPROM_writeAnything(n, touchInterval);
  n = EEPROM_writeAnything(n, touchMask);
  n = EEPROM_writeAnything(n, calibrationData);
}
