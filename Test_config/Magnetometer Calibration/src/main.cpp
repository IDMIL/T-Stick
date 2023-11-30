//****************************************************************************//
// T-Stick - IMU Calibration Firmware                                         //
// Input Devices and Music Interaction Laboratory (IDMIL), McGill University  //
// Albert-Ngabo Niyonsenga (2023)                                             //
//****************************************************************************//

/*
This firmware spits out serial data for the acceleration, magnetometer and gyroscope 
to be used with MotionCal firmware for IMU Calibration.
 */

#include "USB.h"
#include <deque>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <vector>

// I2C
#define SDA_PIN 12
#define SCL_PIN 11
#define I2CUPDATE_FREQ 400000


////////////////////////////////
// Include IMU function files //
////////////////////////////////

#include "ICM_20948.h"
ICM_20948_I2C imu;

#define WIRE_PORT Wire // Your desired Wire port.      Used when "USE_SPI" is not defined
// The value of the last bit of the I2C address.
// On the SparkFun 9DoF IMU breakout the default is 1, and when the ADR jumper is closed the value becomes 0
#define AD0_VAL 0

void initIMU() {
    // Initialise IMU based on Sparkfun IMC20948 library Advanced Example
    // https://github.com/sparkfun/SparkFun_ICM-20948_ArduinoLibrary/blob/main/examples/Arduino/Example2_Advanced/Example2_Advanced.ino
    imu.begin(WIRE_PORT,AD0_VAL);

    // Reset IMU so it is in known stable
    imu.swReset();
    delay(200);

    // wake up IMu
    imu.sleep(false);
    imu.lowPower(false);

    // set continuous sample mode
    imu.setSampleMode((ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr), ICM_20948_Sample_Mode_Continuous);

    // Set scale settings
    ICM_20948_fss_t myFSS; // This uses a "Full Scale Settings" structure that can contain values for all configurable sensors
    myFSS.a = gpm8; 
    myFSS.g = dps2000;
    imu.setFullScale((ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr), myFSS);

    // Set up Digital Low-Pass Filter configuration
    ICM_20948_dlpcfg_t myDLPcfg;    // Similar to FSS, this uses a configuration structure for the desired sensors
    myDLPcfg.a = acc_d473bw_n499bw; // (ICM_20948_ACCEL_CONFIG_DLPCFG_e)
                                    // acc_d246bw_n265bw      - means 3db bandwidth is 246 hz and nyquist bandwidth is 265 hz
                                    // acc_d111bw4_n136bw
                                    // acc_d50bw4_n68bw8
                                    // acc_d23bw9_n34bw4
                                    // acc_d11bw5_n17bw
                                    // acc_d5bw7_n8bw3        - means 3 db bandwidth is 5.7 hz and nyquist bandwidth is 8.3 hz
                                    // acc_d473bw_n499bw
    myDLPcfg.g = gyr_d361bw4_n376bw5; // (ICM_20948_GYRO_CONFIG_1_DLPCFG_e)
                                    // gyr_d196bw6_n229bw8
                                    // gyr_d151bw8_n187bw6
                                    // gyr_d119bw5_n154bw3
                                    // gyr_d51bw2_n73bw3
                                    // gyr_d23bw9_n35bw9
                                    // gyr_d11bw6_n17bw8
                                    // gyr_d5bw7_n8bw9
                                    // gyr_d361bw4_n376bw5
    imu.setDLPFcfg((ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr), myDLPcfg);
    ICM_20948_Status_e accDLPEnableStat = imu.enableDLPF(ICM_20948_Internal_Acc, false);
    ICM_20948_Status_e gyrDLPEnableStat = imu.enableDLPF(ICM_20948_Internal_Gyr, false);

    // Enable magnetometer
    imu.startupMagnetometer();
}

// Function for simple gyroscope and accel calibration
std::vector<float> gyroX = {};
std::vector<float> gyroY = {};
std::vector<float> gyroZ = {};

std::vector<float> accelX = {};
std::vector<float> accelY = {};
std::vector<float> accelZ = {};

void calibrate_accel_gyro() {

}

// Simple hard iron magnetometer calibration
void calibrate_magnetometer() {

}

///////////
// setup //
///////////

void setup() {
    // Set up I2C clock
    Wire.begin(SDA_PIN, SCL_PIN);
    Wire.setClock(I2CUPDATE_FREQ); // Fast mode

    std::cout << "    Initializing IMU... ";
    initIMU();
    std::cout << "done" << std::endl;
}

//////////
// loop //
//////////

void loop() {
    // Read IMU data if interrupt received 
    // read IMU data
    imu.getAGMT();

    // read IMU and update puara-gestures
    // In g's
}