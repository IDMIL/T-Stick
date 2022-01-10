// Include BNO080 Libraries

#ifndef BNO080_H
#define BMO080_H

#include "SparkFun_BNO080_Arduino_Library.h" // https://github.com/sparkfun/SparkFun_BNO080_Arduino_Library

class Imu_BNO080 {
    public:
        bool initIMU();
        bool dataAvailable();
        float getAccelX(); // in m/s^2
        float getAccelY();
        float getAccelZ();
        float getGyroX(); // in radians per second
        float getGyroY();
        float getGyroZ();
        float getMagX(); // in uTesla
        float getMagY();
        float getMagZ();
        unsigned int getMagAccuracy();
        float getQuatI();
        float getQuatJ();
        float getQuatK();
        float getQuatReal();
        float getQuatRadianAccuracy();
        float getRoll(); // in degrees
        float getPitch();
        float getYaw();
  };

  #endif