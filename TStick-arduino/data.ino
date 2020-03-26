
void readData() {

  // Read capsense touch data
  for (byte i=0; i < nCapsenses; ++i) {
      capsense = capsenseRequest(capsense_addresses[i],BUTTON_STAT, 2);
      RawData.touch[i][0] = capsense.answer1;
      RawData.touch[i][1] = capsense.answer2;
  }

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
      offsetYaw = RawData.ypr[0];
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


void printData() {
  if (millis() - serialLastRead > serialInterval) {
      serialLastRead = millis(); 
      Serial.println("\nPrinting sensor data: ");
      Serial.print("RawData.touch: ");
      Serial.print(RawData.touch[0][0]); Serial.print(", ");
      Serial.print(RawData.touch[0][1]); Serial.print(", ");
      Serial.print(RawData.touch[1][0]); Serial.print(", ");
      Serial.print(RawData.touch[1][1]); Serial.print(", ");
      Serial.print(RawData.touch[2][0]); Serial.print(", ");
      Serial.print(RawData.touch[2][1]); Serial.print(", ");
      Serial.print(RawData.touch[3][0]); Serial.print(", ");
      Serial.print(RawData.touch[3][1]); Serial.print(", ");
      Serial.print(RawData.touch[4][0]); Serial.print(", ");
      Serial.print(RawData.touch[4][1]); Serial.print(", ");
      // Serial.print(RawData.touch[5][0]); Serial.print(", ");
      // Serial.println(RawData.touch[5][1]);
      // Serial.print("RawData.touch(libmapper): ");
      //   for( int i = 0 ; i < touchtempSize ; ++i ){
      //       Serial.print(touchtemp[i]);
      //       Serial.print(" ");
      //     }
      Serial.println();
      Serial.print("\nRawData.fsr: "); Serial.println(RawData.fsr);
      Serial.print("RawData.piezo: "); Serial.println(RawData.piezo);
      Serial.print("RawData.accl: ");
          for( int i = 0 ; i < (sizeof(RawData.accl)/sizeof(RawData.accl[0])) ; ++i ){
            Serial.print(RawData.accl[i], 10);
            Serial.print(" ");
          }
      Serial.print("\nRawData.gyro: ");
          for( int i = 0 ; i < (sizeof(RawData.gyro)/sizeof(RawData.gyro[0])) ; ++i ){
            Serial.print(RawData.gyro[i], 10);
            Serial.print(" ");
          }
      Serial.print("\nRawData.magn: ");
          for( int i = 0 ; i < (sizeof(RawData.magn)/sizeof(RawData.magn[0])) ; ++i ){
            Serial.print(RawData.magn[i], 10);
            Serial.print(" ");
          }
      Serial.print("\nRawData.raw: ");
          for( int i = 0 ; i < (sizeof(RawData.raw)/sizeof(RawData.raw[0])) ; ++i ){
            Serial.print(RawData.raw[i], 10);
            Serial.print(" ");
          }
      Serial.print("\nRawData.quat: ");
          for( int i = 0 ; i < (sizeof(RawData.quat)/sizeof(RawData.quat[0])) ; ++i ){
            Serial.print(RawData.quat[i], 10);
            Serial.print(" ");
          }
      Serial.print("\nRawData.ypr: ");
          for( int i = 0 ; i < (sizeof(RawData.ypr)/sizeof(RawData.ypr[0])) ; ++i ){
            Serial.print(RawData.ypr[i], 10);
            Serial.print(" ");
          }
      Serial.println();
  }
}
