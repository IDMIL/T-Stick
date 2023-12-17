#ifndef IMU_H
#define IMU_H

#ifdef imu_ICM20948
#include "ICM_20948.h"
#endif
#ifdef imu_LSM9DS1
#include <SparkFunLSM9DS1.h>
#endif

#define WIRE_PORT Wire // Your desired Wire port.      Used when "USE_SPI" is not defined
// The value of the last bit of the I2C address.
// On the SparkFun 9DoF IMU breakout the default is 1, and when the ADR jumper is closed the value becomes 0
#define AD0_VAL 0

class IMU {
    public:
        #ifdef imu_ICM20948
            ICM_20948_I2C icm20948_imu;
        #endif
        #ifdef imu_LSM9DS1
        LSM9DS1 lsm9ds1_imu;
        #endif

        // Methods
        bool initIMU();
        
        // Read data
        void getData();

        // Store data
        float accl[3];
        float gyro[3];
        float magn[3];
};
#endif