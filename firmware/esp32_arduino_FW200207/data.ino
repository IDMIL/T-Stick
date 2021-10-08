
void readData() {

  // Read capsense touch data
  for (byte i=0; i < nCapsenses; ++i) {
      capsense = capsenseRequest(capsense_addresses[i],BUTTON_STAT, 2);
      RawData.touch[i][0] = capsense.answer1;
      RawData.touch[i][1] = capsense.answer2;
  }

  // read FSR
  RawData.fsr = analogRead(fsrPin);
  NormData.fsr = mapfloat(RawData.fsr, Tstick.FSRoffset, 4095, 0, 1);

  // read piezo
  RawData.piezo = analogRead(piezoPin);
  NormData.piezo = mapfloat(RawData.piezo, 0, 1024, 0, 1);

  // read battery level (https://www.youtube.com/watch?v=yZjpYmWVLh8&feature=youtu.be&t=88) 
  battery = analogRead(batteryPin);
  battery =  battery / 4096.0 * 7.445;

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

    //calculateEulerAnglesQuat(RawData.quat[0],RawData.quat[1],RawData.quat[2],RawData.quat[3]);
    calculateEulerAngles(RawData.accl[0],RawData.accl[1],RawData.accl[2],RawData.magn[0],RawData.magn[1],RawData.magn[2]);
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
      printf("%i, %i, %i, %i, %i, %i, %i, %i, %i, %i",
      RawData.touch[0][0],RawData.touch[0][1],
      RawData.touch[1][0],RawData.touch[1][1],
      RawData.touch[2][0],RawData.touch[2][1],
      RawData.touch[3][0],RawData.touch[3][1],
      RawData.touch[4][0],RawData.touch[4][1]);
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


void calculateEulerAnglesQuat(float w, float x, float y, float z) {

    // roll (x-axis rotation)
    float sinr_cosp = 2 * (w * x + y * z);
    float cosr_cosp = 1 - 2 * (x * x + y * y);
    RawData.ypr[2] = std::atan2(sinr_cosp, cosr_cosp);

    // pitch (y-axis rotation)
    float sinp = 2 * (w * y - z * x);
    if (std::abs(sinp) >= 1)
        RawData.ypr[1] = std::copysign(M_PI / 2, sinp); // use 90 degrees if out of range
    else
        RawData.ypr[1] = std::asin(sinp);

    // yaw (z-axis rotation)
    float siny_cosp = 2 * (w * z + x * y);
    float cosy_cosp = 1 - 2 * (y * y + z * z);
    RawData.ypr[0] = std::atan2(siny_cosp, cosy_cosp);
}


void calculateEulerAngles(float ax, float ay, float az, float mx, float my, float mz) {
  ax *= 16; ay *= 16; az *= 16;
  mx *= 8; my *= 8; mz *= 8;
  RawData.ypr[2] = std::atan2(ay, az);
  RawData.ypr[1] = std::atan2(-ax, sqrt(ay * ay + az * az));
  if (my == 0)
    RawData.ypr[0] = (mx < 0) ? PI : 0;
  else
    RawData.ypr[0] = std::atan2(mx, my);

  RawData.ypr[0] -= -14.34 * PI / 180; // Declination (degrees) in Montreal, QC.

  if (RawData.ypr[0] > PI) RawData.ypr[0] -= (2 * PI);
  else if (RawData.ypr[0] < -PI) RawData.ypr[0] += (2 * PI);

  // Convert everything from radians to degrees:
  RawData.ypr[0] *= 180.0 / PI;
  RawData.ypr[1] *= 180.0 / PI;
  RawData.ypr[2]  *= 180.0 / PI;
}
