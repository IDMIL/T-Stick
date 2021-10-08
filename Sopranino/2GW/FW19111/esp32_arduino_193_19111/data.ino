
void readData() {

  // read capsense
//  if (millis() - touchLastRead > touchInterval) {
//      touchLastRead = millis();
//      readTouch();
//  }

  // read FSR
  Data.fsr = analogRead(fsrPin);
  Data.fsr = mapfloat(Data.fsr, Tstick.FSRoffset, 4095, 0, 1);

  // read piezo
  Data.piezo = analogRead(piezoPin);
  Data.piezo = mapfloat(Data.piezo, 0, 1024, 0, 1);

  // read IMU
  static MIMUReading reading = MIMUReading::Zero();
  static Quaternion quat = Quaternion::Identity();
  if (mimu.readInto(reading)) {
    reading.updateBuffer();
    copyFloatArrayToVar(reading.data, reading.size, Data.raw); //read raw before calibrate
    calibrator.calibrate(reading);
    quat = filter.fuse(reading.gyro, reading.accl, reading.magn);
    
    copyFloatArrayToVar(reading.accl.data(), reading.accl.size(), Data.accl);
    copyFloatArrayToVar(reading.gyro.data(), reading.gyro.size(), Data.gyro);
    copyFloatArrayToVar(reading.magn.data(), reading.magn.size(), Data.magn);
    copyFloatArrayToVar(quat.coeffs().data(), quat.coeffs().size(), Data.quat);
    
//    for (int i = 0; i < (sizeof(Data.accl)/sizeof(Data.accl[0])); i++) {
//      Data.accl[i] = mapfloat(Data.accl[i], -32767, 32767, -1, 1);
//      }
//    for (int i = 0; i < (sizeof(Data.gyro)/sizeof(Data.gyro[0])); i++) {
//      Data.gyro[i] = mapfloat(Data.gyro[i], -34.90659, 34.90659, -1, 1);
//      }
//    for (int i = 0; i < (sizeof(Data.magn)/sizeof(Data.magn[0])); i++) {
//      Data.magn[i] = mapfloat(Data.magn[i], -32767, 32767, -1, 1);
//      }

    //calculateEulerAnglesQuat(Data.quat[0],Data.quat[1],Data.quat[2],Data.quat[3]);
    calculateEulerAngles(Data.accl[0],Data.accl[1],Data.accl[2],Data.magn[0],Data.magn[1],Data.magn[2]);
  } 
}


boolean readTouch(){
  boolean changed = 0;
  byte temp[2] = {0, 0}; int i=0;
  Wire.beginTransmission(I2C_ADDR);
  Wire.write(BUTTON_STAT);
  Wire.endTransmission();

  Wire.requestFrom(I2C_ADDR,2);
  while (Wire.available()) { // slave may send less than requested
    temp[i] = Wire.read(); // receive a byte as character
    i++;
  }    
  Wire.endTransmission();

  for (int t = 0; t<2; t++){
    if (temp[t] != Data.touch[t]){
      changed = 1;
      Data.touch[t] = temp[t];
    }
  }
  return changed;
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
      Serial.print("Data.touch: ");
        for( int i = 0 ; i < (sizeof(Data.touch)/sizeof(Data.touch[0])) ; ++i ){
          Serial.print(Data.touch[i], 10);
          Serial.print(" ");
        }
    Serial.print("\nData.fsr: "); Serial.println(Data.fsr);
    Serial.print("Data.piezo: "); Serial.println(Data.piezo);
    Serial.print("Data.accl: ");
        for( int i = 0 ; i < (sizeof(Data.accl)/sizeof(Data.accl[0])) ; ++i ){
          Serial.print(Data.accl[i], 10);
          Serial.print(" ");
        }
    Serial.print("\nData.gyro: ");
        for( int i = 0 ; i < (sizeof(Data.gyro)/sizeof(Data.gyro[0])) ; ++i ){
          Serial.print(Data.gyro[i], 10);
          Serial.print(" ");
        }
    Serial.print("\nData.magn: ");
        for( int i = 0 ; i < (sizeof(Data.magn)/sizeof(Data.magn[0])) ; ++i ){
          Serial.print(Data.magn[i], 10);
          Serial.print(" ");
        }
    Serial.print("\nData.raw: ");
        for( int i = 0 ; i < (sizeof(Data.raw)/sizeof(Data.raw[0])) ; ++i ){
          Serial.print(Data.raw[i], 10);
          Serial.print(" ");
        }
    Serial.print("\nData.quat: ");
        for( int i = 0 ; i < (sizeof(Data.quat)/sizeof(Data.quat[0])) ; ++i ){
          Serial.print(Data.quat[i], 10);
          Serial.print(" ");
        }
    Serial.print("\nData.ypr: ");
        for( int i = 0 ; i < (sizeof(Data.ypr)/sizeof(Data.ypr[0])) ; ++i ){
          Serial.print(Data.ypr[i], 10);
          Serial.print(" ");
        }
    Serial.println();
  }
}


void calculateEulerAnglesQuat(float w, float x, float y, float z) {

    // roll (x-axis rotation)
    float sinr_cosp = 2 * (w * x + y * z);
    float cosr_cosp = 1 - 2 * (x * x + y * y);
    Data.ypr[2] = std::atan2(sinr_cosp, cosr_cosp);

    // pitch (y-axis rotation)
    float sinp = 2 * (w * y - z * x);
    if (std::abs(sinp) >= 1)
        Data.ypr[1] = std::copysign(M_PI / 2, sinp); // use 90 degrees if out of range
    else
        Data.ypr[1] = std::asin(sinp);

    // yaw (z-axis rotation)
    float siny_cosp = 2 * (w * z + x * y);
    float cosy_cosp = 1 - 2 * (y * y + z * z);
    Data.ypr[0] = std::atan2(siny_cosp, cosy_cosp);
}


void calculateEulerAngles(float ax, float ay, float az, float mx, float my, float mz) {
  ax *= 16; ay *= 16; az *= 16;
  mx *= 8; my *= 8; mz *= 8;
  Data.ypr[2] = atan2(ay, az);
  Data.ypr[1] = atan2(-ax, sqrt(ay * ay + az * az));
  if (my == 0)
    Data.ypr[0] = (mx < 0) ? PI : 0;
  else
    Data.ypr[0] = atan2(mx, my);

  Data.ypr[0] -= -14.34 * PI / 180; // Declination (degrees) in Montreal, QC.

  if (Data.ypr[0] > PI) Data.ypr[0] -= (2 * PI);
  else if (Data.ypr[0] < -PI) Data.ypr[0] += (2 * PI);

  // Convert everything from radians to degrees:
  Data.ypr[0] *= 180.0 / PI;
  Data.ypr[1] *= 180.0 / PI;
  Data.ypr[2]  *= 180.0 / PI;
}
