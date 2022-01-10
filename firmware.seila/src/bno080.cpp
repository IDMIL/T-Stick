// translation file between library commands for IMU

#include "bno080.h"

BNO080 bno080; // initialize library class

// Initializing IMU
    bool Imu_BNO080::initIMU() {
        Wire.begin();
        bno080.begin();
        Wire.setClock(400000);             // Increase I2C data rate to 400kHz

        // We must enable the accel in order to get MEMS readings even if we don't read the reports.
        bno080.enableAccelerometer(100);     // Send data update every 100ms, Output in m/s^2
        bno080.enableGyro(100);              // Send data update every 100ms, Output in radians per second
        bno080.enableGameRotationVector(100);// Send data update every 100ms
        bno080.enableMagnetometer(100);
        bno080.calibrateAll();               // Turn on cal for Accel, Gyro, and Mag
                                            // In general, calibration should be left on at all times. The BNO080
                                            //auto-calibrates and auto-records cal data roughly every 5 minutes


        return true;
    }

// Read IMU data

  bool Imu_BNO080::dataAvailable() {
      return bno080.dataAvailable(); // in m/s^2
  };

  float Imu_BNO080::getAccelX() {
      return bno080.getAccelX();
  };

  float Imu_BNO080::getAccelY() {
      return bno080.getAccelY();
  };

  float Imu_BNO080::getAccelZ() {
      return bno080.getAccelZ();
  };

  float Imu_BNO080::getGyroX() {
      return bno080.getGyroX(); // in radians per second
  };

  float Imu_BNO080::getGyroY() {
      return bno080.getGyroY();
  };

  float Imu_BNO080::getGyroZ() {
      return bno080.getGyroZ();
  };

  float Imu_BNO080::getMagX() {
      return bno080.getMagX(); // in uTesla
  };

  float Imu_BNO080::getMagY() {
      return bno080.getMagY();
  };

  float Imu_BNO080::getMagZ() {
      return bno080.getMagZ();
  };

  unsigned int Imu_BNO080::getMagAccuracy() {
      return bno080.getMagAccuracy();
  };

  float Imu_BNO080::getQuatI() {
      return bno080.getQuatI();
  };

  float Imu_BNO080::getQuatJ() {
      return bno080.getQuatJ();
  };

  float Imu_BNO080::getQuatK() {
      return bno080.getQuatK();
  };

  float Imu_BNO080::getQuatReal() {
      return bno080.getQuatReal();
  };

  float Imu_BNO080::getQuatRadianAccuracy() {
      return bno080.getQuatRadianAccuracy();
  };

float Imu_BNO080::getRoll() {
      return (bno080.getRoll()) * 180.0 / PI; // Convert roll to degrees
  };

float Imu_BNO080::getPitch() {
      return (bno080.getPitch()) * 180.0 / PI; // Convert pitch to degrees
  };

float Imu_BNO080::getYaw() {
      return (bno080.getYaw()) * 180.0 / PI; // Convert yaw / heading to degrees
  };
