#ifndef IMU_H
#define IMU_H

#include "tstick-sensors.h"
#include "ICM_20948.h"
#include <SparkFunLSM9DS1.h>


struct imu_config {
    // Device
    board_IMU imu_board
};

class IMU {
    public:
        ICM_20948_I2C icm20948_imu;
        LSM9DS1 lsm9ds1_imu;
        int imuboard = board_IMU::imu_ICM20948;

        // Methods
        bool initIMU(imu_config config);
        
        // Read data
        void getData();

        // Store data
        float accl[3];
        float gyro[3];
        float magn[3];
}