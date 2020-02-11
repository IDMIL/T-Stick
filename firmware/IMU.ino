

void initIMU() {

  Serial.println("\nConfiguring IMU and MIMU library...");

  mimu.setup();
  calibrator.cc = MIMUCalibrationConstants{}; // reset calibration

  calibrator.setup();
  //filter.fc = MIMUFilterCoefficients{3,0,1,1};
  filter.setup();

  Serial.println("IMU/MIMU configuration complete");
}
