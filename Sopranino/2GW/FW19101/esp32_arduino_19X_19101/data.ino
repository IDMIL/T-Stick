
void readData() {

  // read capsense
  if (millis() - touchLastRead > touchInterval) {
      touchLastRead = millis();
      readTouch();
  }

  // read FSR
  Data.fsr = analogRead(fsrPin);
  if (Tstick.FSRcalibration == 1) {
    Data.fsr = map(Data.fsr, Tstick.FSRcalibrationValues[0], Tstick.FSRcalibrationValues[1], 0, 4095);
    if (Data.fsr < 0) {Data.fsr = 0;}
  }

  // read piezo
  Data.piezo = analogRead(piezoPin);

  // read IMU
  static MIMUReading reading = MIMUReading::Zero();
  static Quaternion quat = Quaternion::Identity();
  if (mimu.readInto(reading)) {
    calibrator.calibrate(reading);
    reading.updateBuffer();
    quat = filter.fuse(reading.gyro, reading.accl, reading.magn);
    copyFloatArrayToVar(reading.accl.data(), reading.accl.size(), Data.accl);
    copyFloatArrayToVar(reading.gyro.data(), reading.gyro.size(), Data.gyro);
    copyFloatArrayToVar(reading.magn.data(), reading.magn.size(), Data.magn);
    copyFloatArrayToVar(reading.data, reading.size, Data.raw);
    copyFloatArrayToVar(quat.coeffs().data(), quat.coeffs().size(), Data.quat);
    Data.gyro_map[0] = mapfloat(Data.gyro[0], -5.00421, 5.00344, -32764.00, 32764.00);
    Data.gyro_map[1] = mapfloat(Data.gyro[1], -5.00421, 5.00344, -32764.00, 32764.00);
    Data.gyro_map[2] = mapfloat(Data.gyro[2], -5.00421, 5.00344, -32764.00, 32764.00);
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
    // byte c = Wire.read();
    // temp[i] = c; // receive a byte as character
    temp[i] = Wire.read();
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

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
