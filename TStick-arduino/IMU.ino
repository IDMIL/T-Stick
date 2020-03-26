

void initIMU() {

  Serial.println("\nConfiguring IMU and MIMU library...");

  // Check MIMU_LSM9DS1.cpp to change IMU settings

  mimu.setup();
  calibrator.cc = MIMUCalibrationConstants{}; // reset calibration
  calibrator.setup();
  
  filter.fc = MIMUFilterCoefficients{3,0,1,0};
  filter.setup();

  Serial.println("IMU/MIMU configuration complete");
}

// variables to apply temporary offset
float offsetYaw = 0;
unsigned long offsetDebounce = 0;
byte offsetFlag = 0;

void taitBryanAngles(float w, float x, float y, float z) {
    RawData.ypr[2] = atan2(2.0f * (x * y + w * z), w * w + x * x - y * y - z * z);   
    RawData.ypr[1] = -asin(2.0f * (x * z - w * y));
    RawData.ypr[0] = atan2(2.0f * (w * x + y * z), w * w - x * x - y * y + z * z);
    RawData.ypr[2] *= 180.0f / PI; 
    RawData.ypr[2] -= 14.14; // Declination at Montreal on 2020-03-12
    RawData.ypr[1] *= 180.0f / PI;
    RawData.ypr[0] *= 180.0f / PI;

    // Apply temporary offset
    if (offsetYaw > 0) {
        RawData.ypr[0] -= offsetYaw;
      } else {
        RawData.ypr[0] += offsetYaw;
      }
}

  
