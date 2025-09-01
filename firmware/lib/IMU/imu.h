#ifndef IMU_H
#define IMU_H

#include "ICM_20948.h"
#include <SparkFunLSM9DS1.h>

#define WIRE_PORT Wire // Your desired Wire port.      Used when "USE_SPI" is not defined
// The value of the last bit of the I2C address.
// On the SparkFun 9DoF IMU breakout the default is 1, and when the ADR jumper is closed the value becomes 0
#define AD0_VAL 0

enum MIMUBOARD {
    mimu_ICM20948 = 0,
    mimu_LSM9DS1 = 1,
};

class IMU {
    public:
        ICM_20948_I2C icm20948_imu;
        LSM9DS1 lsm9ds1_imu;

        // imu board
        int mimu_board = MIMUBOARD::mimu_ICM20948;

        // Methods
        bool initIMU(int board = MIMUBOARD::mimu_ICM20948);
        
        // Read data
        void getData();
        void sleep();
        void clearInterrupt();

        // Store data
        float accl[3];
        float gyro[3];
        float magn[3];
};
#endif