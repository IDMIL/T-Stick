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

// Calibration Settings
#define NUMBER_SAMPLES 1000
// #define SIMPLE_CAL // Simple calibration for accelerometer, gyroscope and magnetometer
#define ADVANCED_CAL // Advanced calibration for only magnetometer, sends serial out for motion cal

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

// Calibrate Accelerometer
void calibrate_accelerometer() {
    float min_x, max_x, mid_x;
    float min_y, max_y, mid_y;
    float min_z, max_z, mid_z;

    // Get samples
    for (int i; i < NUMBER_SAMPLES; i++) {
        if (i % 100 == 0) {
            Serial.print("Sample ");
            Serial.println(i);
        }
        imu.getAGMT();
        // Get Accelerometer
        float x = imu.accX() / 1000;
        float y = imu.accY() / 1000;
        float z = imu.accZ() / 1000;

        // Compute min, max and mid point
        min_x = min(min_x, x);
        min_y = min(min_y, y);
        min_z = min(min_z, z);

        max_x = max(max_x, x);
        max_y = max(max_y, y);
        max_z = max(max_z, z);

        mid_x = (max_x + min_x) / 2;
        mid_y = (max_y + min_y) / 2;
        mid_z = (max_z + min_z) / 2;  

        // Add delay between reads
        delay(10);
    }
    Serial.println(F("\n\nFinal zero g offset in g: "));
    Serial.print(mid_x, 4); Serial.print(", ");
    Serial.print(mid_y, 4); Serial.print(", ");
    Serial.println(mid_z, 4);
}

// Calibrate gyroscope
void calibrate_gyroscope() {
    float min_x, max_x, mid_x;
    float min_y, max_y, mid_y;
    float min_z, max_z, mid_z;

    // Get samples
    for (int i; i < NUMBER_SAMPLES; i++) {
        if (i % 100 == 0) {
            Serial.print("Sample ");
            Serial.println(i);
        }

        imu.getAGMT();
        // Get Gyroscope
        float x = imu.gyrX() * M_PI / 180;
        float y = imu.gyrY() * M_PI / 180;
        float z = imu.gyrZ() * M_PI / 180;

        // Compute min, max and mid point
        min_x = min(min_x, x);
        min_y = min(min_y, y);
        min_z = min(min_z, z);

        max_x = max(max_x, x);
        max_y = max(max_y, y);
        max_z = max(max_z, z);

        mid_x = (max_x + min_x) / 2;
        mid_y = (max_y + min_y) / 2;
        mid_z = (max_z + min_z) / 2;

        // Add delay between reads
        delay(10);
    }
    Serial.println(F("\n\nFinal zero rate offset in radians/s: "));
    Serial.print(mid_x, 4); Serial.print(", ");
    Serial.print(mid_y, 4); Serial.print(", ");
    Serial.println(mid_z, 4);
}

// Simple hard iron magnetometer calibration
void calibrate_magnetometer() {
    float min_x, max_x, mid_x;
    float min_y, max_y, mid_y;
    float min_z, max_z, mid_z;
    imu.getAGMT();

    for (int i; i < NUMBER_SAMPLES; i++) {
        if (i % 100 == 0) {
            Serial.print("Sample ");
            Serial.println(i);
        }
        float x = imu.magX();
        float y = imu.magY();
        float z = imu.magZ();

        Serial.print("Mag: (");
        Serial.print(x); Serial.print(", ");
        Serial.print(y); Serial.print(", ");
        Serial.print(z); Serial.print(")");

        min_x = min(min_x, x);
        min_y = min(min_y, y);
        min_z = min(min_z, z);

        max_x = max(max_x, x);
        max_y = max(max_y, y);
        max_z = max(max_z, z);

        mid_x = (max_x + min_x) / 2;
        mid_y = (max_y + min_y) / 2;
        mid_z = (max_z + min_z) / 2;

        Serial.print(" Hard offset: (");
        Serial.print(mid_x); Serial.print(", ");
        Serial.print(mid_y); Serial.print(", ");
        Serial.print(mid_z); Serial.print(")");  

        Serial.print(" Field: (");
        Serial.print((max_x - min_x)/2); Serial.print(", ");
        Serial.print((max_y - min_y)/2); Serial.print(", ");
        Serial.print((max_z - min_z)/2); Serial.println(")");    
        delay(10); 
    }
    Serial.println(F("\n\nFinal Hard offset: "));
    Serial.print(mid_x, 4); Serial.print(", ");
    Serial.print(mid_y, 4); Serial.print(", ");
    Serial.println(mid_z, 4);
}

// Send to motion cal
void advanced_calibration() {
    // 'Raw' values to match expectation of MOtionCal
    imu.getAGMT();

    Serial.print("Raw:");
    Serial.print(int(imu.accX() / 1000)); Serial.print(",");
    Serial.print(int(imu.accY() / 1000)); Serial.print(",");
    Serial.print(int(imu.accZ() / 1000)); Serial.print(",");
    Serial.print(int(imu.gyrX())); Serial.print(",");
    Serial.print(int(imu.gyrY())); Serial.print(",");
    Serial.print(int(imu.gyrZ())); Serial.print(",");
    Serial.print(int(imu.magX())); Serial.print(",");
    Serial.print(int(imu.magY())); Serial.print(",");
    Serial.print(int(imu.magZ())); Serial.println("");

    // unified data
    Serial.print("Uni:");
    Serial.print(int(imu.accX() / 1000)); Serial.print(",");
    Serial.print(int(imu.accY() / 1000)); Serial.print(",");
    Serial.print(int(imu.accZ() / 1000)); Serial.print(",");
    Serial.print(int(imu.gyrX())); Serial.print(",");
    Serial.print(int(imu.gyrY())); Serial.print(",");
    Serial.print(int(imu.gyrZ())); Serial.print(",");
    Serial.print(int(imu.magX())); Serial.print(",");
    Serial.print(int(imu.magY())); Serial.print(",");
    Serial.print(int(imu.magZ())); Serial.println("");
}
///////////
// setup //
///////////

void setup() {
    Serial.begin(112500);
    // Set up I2C clock
    Wire.begin(SDA_PIN, SCL_PIN);
    Wire.setClock(I2CUPDATE_FREQ); // Fast mode

    
    while(!Serial) {
        // Wait for serial port to be available
    }

    std::cout << "    Initializing IMU... ";
    initIMU();
    std::cout << "done" << std::endl;
}

//////////
// loop //
//////////

void loop() {
    // Calibrate Accelerometer first
    #ifdef SIMPLE_CAL
    Serial.print("Calibrating Accelerometer");
    calibrate_accelerometer();
    delay(500);
    Serial.print("Calibrating Gyroscope");
    calibrate_gyroscope();
    delay(500);
    Serial.print("Calibrating Magnetometer");
    calibrate_magnetometer();
    #endif
    #ifdef ADVANCED_CAL
    advanced_calibration();
    #endif

    // read IMU and update puara-gestures
    // In g's
}