// Include LSM9DS1 Libraries

// Settings in lsm9ds1.cpp need to be changed in LSM9DS1::init()
// at SparkFunLSM9DS1.cpp file, since LSM9DS1::init() overrites all 
// custom settings. Check bug report in
// https://github.com/sparkfun/SparkFun_LSM9DS1_Arduino_Library/issues/27

#ifndef LSM9DS1_H
#define LSM9DS1_H

#include <SparkFunLSM9DS1.h> // https://github.com/sparkfun/SparkFun_LSM9DS1_Arduino_Library

#include <cmath>

class Imu_LSM9DS1 {
    public:
        bool initIMU();
        bool dataAvailable();
        float getAccelX(); // in m/s^2, converted from g's
        float getAccelY();
        float getAccelZ();
        float getGyroX(); // in radians per second, converted from DPS
        float getGyroY();
        float getGyroZ();
        float getMagX(); // in uTesla, converted from Gauss
        float getMagY();
        float getMagZ();
        unsigned int getMagAccuracy();
        float getQuatI();
        float getQuatJ();
        float getQuatK();
        float getQuatReal();
        float getQuatRadianAccuracy();
        float getRoll();
        float getPitch();
        float getYaw();
    private:
        void MadgwickQuaternionUpdate(float ax, float ay, float az, float gx, float gy, float gz, float mx, float my, float mz);
        void taitBryanAngles(float w, float x, float y, float z);
        float accelX = 0.0; // in m/s^2, converted from g's
        float accelY = 0.0;
        float accelZ = 0.0;
        float gyroX = 0.0; // in radians per second, converted from DPS
        float gyroY = 0.0;
        float gyroZ = 0.0;
        float magX = 0.0; // in uTesla, converted from Gauss
        float magY = 0.0;
        float magZ = 0.0;
        float q1 = 1.0; // needs to be set to 1 for the first loop, otherwise NaN
        float q2 = 0.0;
        float q3 = 0.0;
        float q4 = 0.0;
        float roll = 0.0;
        float pitch = 0.0;
        float yaw = 0.0;
        float declination = -14.14; // Declination at Montreal on 2020-03-12
        double pi = 3.141592653589793238462643383279502884;
        uint32_t lastUpdate = 0;    // used to calculate integration interval
        uint32_t Now = 0;           // used to calculate integration interval
        float deltat = 0.0;        // integration interval for both filter schemes
};

  #endif