
#include "TStick.h"

void readData() {
  
  // Read capsense touch data
  for (byte i=0; i < nCapsenses; ++i) {
      capsense = capsenseRequest(capsense_addresses[i],BUTTON_STAT, 2);
      RawData.touch[i*2] = capsense.answer1;
      RawData.touch[(i*2)+1] = capsense.answer2;
  }
  for (byte i=0; i < touchStripsSize; ++i) {
      RawData.touchStrips[i] = bitRead(RawData.touch[i/8],7-(i%8));
  }
  reorderCapsense (RawData.touchStrips, touchStripsSize);

  // Read button
  buttonState = !digitalRead(buttonPin);
  if (buttonState == 0 && buttonLongFlag == 0) { // Read button (long)
    RawData.buttonLong = 0;
    RawData.buttonShort = 0;
  }
  if (buttonState == 1 && buttonLongFlag == 0) { 
    buttonTimer = millis(); 
    buttonLongFlag = buttonShortFlag = 1;
  }
  if (millis() - buttonShortInterval > buttonTimer && buttonShortFlag == 1) {
    RawData.buttonShort = 1;
    buttonShortFlag = 0;
  }
  if (millis() - buttonLongInterval > buttonTimer && buttonLongFlag == 1) { 
    RawData.buttonLong = 1;
  }
  if (buttonState == 0 && buttonLongFlag == 1) {
     buttonLongFlag = 0;
  }
  if (buttonState == 1 && buttonDoubleFlag == 0) { // Read button (double)
    buttonDoubleTimer = millis();
    buttonDoubleFlag = 1;
  }
  if (millis() - buttonShortInterval > buttonDoubleTimer) { 
    if (buttonState == 0) {
      buttonDoubleFlag = 0;
      RawData.buttonDouble = 0;
    } 
  } else {
    if (buttonState == 0) {
      buttonDoubleFlag = 2;
    }
    if (buttonState == 1 && buttonDoubleFlag == 2) {
      RawData.buttonDouble = 1;
      buttonShortFlag = 0;
    }
  }


  // read FSR
  RawData.fsr = analogRead(fsrPin);
  NormData.fsr = mapfloat(RawData.fsr, Tstick.FSRoffset, 4095, 0, 1);

  // read piezo
  RawData.piezo = analogRead(piezoPin);
  NormData.piezo = mapfloat(RawData.piezo, 0, 1024, 0, 1);

  // read battery level (https://www.youtube.com/watch?v=yZjpYmWVLh8&feature=youtu.be&t=88) 
  if (millis() - batteryLastRead > batteryInterval) {
      batteryCount += 1;
      batteryLastRead = millis(); 
      battery += (analogRead(batteryPin) / 4096.0 * 7.445) / 10;
      if (batteryCount >= 10) {
        if ( battery < 2.9 ) {
          batteryPercentage = 5;
        }
        else if ( battery > 4.15 ) {
          batteryPercentage = 100;
        }
        else {
          batteryPercentage = mapfloat(battery, 2.9, 4.15, 5.0, 99.0);
        }
        battery = 0;
        batteryCount = 0;
        }
      }

  // read IMU
  static MIMUReading reading = MIMUReading::Zero();
  static Quaternion quat = Quaternion::Identity();
  if (mimu.readInto(reading)) {
    calibrator.calibrate(reading);
    reading.updateBuffer();
    quat = filter.fuse(reading.gyro, reading.accl, reading.magn);
    
    copyFloatArrayToVar(reading.accl.data(), reading.accl.size(), RawData.accl);
    copyFloatArrayToVar(reading.gyro.data(), reading.gyro.size(), RawData.gyro);
    copyFloatArrayToVar(reading.magn.data(), reading.magn.size(), RawData.magn);
    copyFloatArrayToVar(reading.data, reading.size, RawData.raw);
    copyFloatArrayToVar(quat.coeffs().data(), quat.coeffs().size(), RawData.quat);

    LastState.gyroXArray[LastState.gyroArrayCounter] = RawData.gyro[0];
    LastState.gyroYArray[LastState.gyroArrayCounter] = RawData.gyro[1];
    LastState.gyroZArray[LastState.gyroArrayCounter] = RawData.gyro[2];
    if (LastState.gyroArrayCounter < 5) {
      LastState.gyroArrayCounter++;
    }
    else {
      LastState.gyroArrayCounter = 0;
    }
    
    for (int i = 0; i < (sizeof(RawData.accl)/sizeof(RawData.accl[0])); ++i) {
      NormData.accl[i] = mapfloat(RawData.accl[i], -32767, 32767, -1, 1);
      }
    for (int i = 0; i < (sizeof(RawData.gyro)/sizeof(RawData.gyro[0])); ++i) {
      NormData.gyro[i] = mapfloat(RawData.gyro[i], -41, 41, -1, 1);
      }
    for (int i = 0; i < (sizeof(RawData.magn)/sizeof(RawData.magn[0])); ++i) {
      NormData.magn[i] = mapfloat(RawData.magn[i], -32767, 32767, -1, 1);
      }

    RawData.magAccl = sqrt(RawData.accl[0] * RawData.accl[0] + RawData.accl[1] * RawData.accl[1] + RawData.accl[2] * RawData.accl[2]);
    RawData.magGyro = sqrt(RawData.gyro[0] * RawData.gyro[0] + RawData.gyro[1] * RawData.gyro[1] + RawData.gyro[2] * RawData.gyro[2]);
    RawData.magMagn = sqrt(RawData.magn[0] * RawData.magn[0] + RawData.magn[1] * RawData.magn[1] + RawData.magn[2] * RawData.magn[2]);

    taitBryanAngles(RawData.quat[0],RawData.quat[1],RawData.quat[2],RawData.quat[3]);

    // Apply temporary Yaw offset
    if (RawData.buttonDouble == 1 && millis() - 300 > offsetDebounce) {
      offsetDebounce = millis();
      offsetYaw = 0;
      offsetFlag = 1;
    }
    if (offsetFlag == 1 && millis() - 200 > offsetDebounce) {
      offsetYaw = InstrumentData.ypr[0];
      offsetFlag = 0;
    }
  } 
}

void copyFloatArrayToVar(const float source[], int size, float destination[]) {
  for (int i = 0; i < size; ++i) {
    destination[i] = source[i];
  }
}


float mapfloat(float x, float in_min, float in_max, float out_min, float out_max) {
  float result = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
  result = constrain(result, out_min, out_max);
  return result;
}


void reorderCapsense (byte *origArray, byte arraySize) {
  byte tempArray[arraySize];
  byte order[64] = {
    8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,
    24,25,26,27,28,29,30,31,16,17,18,19,20,21,22,23,
    40,41,42,43,44,45,46,47,32,33,34,35,36,37,38,39,
    56,57,58,59,60,61,62,63,48,49,50,51,52,53,54,55
  };
  for (byte i=0; i < sizeof(tempArray)/sizeof(tempArray[0]); ++i) {
    tempArray[i] = origArray[order[i]];
  }
  memcpy(origArray, tempArray, sizeof(tempArray));
}


void printData() {
  // if (millis() - serialLastRead > serialInterval) {
  //     serialLastRead = millis(); 
  //     Serial.println("\nPrinting sensor data: \n");
  //     Serial.print("RawData.touch: ");
  //     for (byte i=0; i < sizeof(RawData.touch); ++i) { //for (byte i=0; i < nCapsenses*2; ++i) {
  //       Serial.print(RawData.touch[i]); Serial.print(" ");
  //     }
  //     Serial.println();
  //     Serial.print("RawData.touch(libmapper): ");
  //       for( int i = 0 ; i < touchStripsSize ; ++i ){
  //           Serial.print(RawData.touchStrips[i]);
  //           Serial.print(" ");
  //         }
  //     Serial.print("\nRawData.fsr: "); Serial.println(RawData.fsr);
  //     Serial.print("RawData.piezo: "); Serial.println(RawData.piezo);
  //     Serial.print("RawData.accl: ");
  //         for( int i = 0 ; i < (sizeof(RawData.accl)/sizeof(RawData.accl[0])) ; ++i ){
  //           Serial.print(RawData.accl[i], 10);
  //           Serial.print(" ");
  //         }
  //     Serial.print("\nRawData.gyro: ");
  //         for( int i = 0 ; i < (sizeof(RawData.gyro)/sizeof(RawData.gyro[0])) ; ++i ){
  //           Serial.print(RawData.gyro[i], 10);
  //           Serial.print(" ");
  //         }
  //     Serial.print("\nRawData.magn: ");
  //         for( int i = 0 ; i < (sizeof(RawData.magn)/sizeof(RawData.magn[0])) ; ++i ){
  //           Serial.print(RawData.magn[i], 10);
  //           Serial.print(" ");
  //         }
  //     Serial.print("\nRawData.raw: ");
  //         for( int i = 0 ; i < (sizeof(RawData.raw)/sizeof(RawData.raw[0])) ; ++i ){
  //           Serial.print(RawData.raw[i], 10);
  //           Serial.print(" ");
  //         }
  //     Serial.print("\nRawData.quat: ");
  //         for( int i = 0 ; i < (sizeof(RawData.quat)/sizeof(RawData.quat[0])) ; ++i ){
  //           Serial.print(RawData.quat[i], 10);
  //           Serial.print(" ");
  //         }
  //     Serial.print("\nInstrumentData.ypr: ");
  //         for( int i = 0 ; i < (sizeof(InstrumentData.ypr)/sizeof(InstrumentData.ypr[0])) ; ++i ){
  //           Serial.print(InstrumentData.ypr[i], 10);
  //           Serial.print(" ");
  //         }
  //     Serial.println();
      
  //     Serial.println("\nPrinting Instrument data: ");
  //     Serial.print("\nInstrumentData.touchAll: "); Serial.println(InstrumentData.touchAll);
  //     Serial.print("InstrumentData.touchTop: "); Serial.println(InstrumentData.touchTop);
  //     Serial.print("InstrumentData.touchMiddle: "); Serial.println(InstrumentData.touchMiddle);
  //     Serial.print("InstrumentData.touchBottom: "); Serial.println(InstrumentData.touchBottom);

  //     Serial.print("\nBlobDetection.blobArray: ");
  //         for( int i = 0 ; i < (sizeof(BlobDetection.blobArray)/sizeof(BlobDetection.blobArray[0])) ; ++i ){
  //           Serial.print(BlobDetection.blobArray[i]);
  //           Serial.print(" ");
  //         }
  //     Serial.println();
  //     Serial.print("\nBlobDetection.blobPos: ");
  //         for( int i = 0 ; i < (sizeof(BlobDetection.blobPos)/sizeof(BlobDetection.blobPos[0])) ; ++i ){
  //           Serial.print(BlobDetection.blobPos[i]);
  //           Serial.print(" ");
  //         }
  //     Serial.println();
  //     Serial.print("\nBlobDetection.blobSize: ");
  //         for( int i = 0 ; i < (sizeof(BlobDetection.blobSize)/sizeof(BlobDetection.blobSize[0])) ; ++i ){
  //           Serial.print(BlobDetection.blobSize[i], 4);
  //           Serial.print(" ");
  //         }
  //     Serial.println();

  //     Serial.print("InstrumentData.brush: "); Serial.println(InstrumentData.brush);

      
  //     Serial.println();
  // }
}
