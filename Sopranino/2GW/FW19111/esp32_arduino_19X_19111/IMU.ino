

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
