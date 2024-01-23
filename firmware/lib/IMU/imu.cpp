#include "imu.h"

bool IMU::initIMU(int board) {
    // Get IMU config
    mimu_board = board;
    if (mimu_board == MIMUBOARD::mimu_ICM20948) {
        // Initialise IMU based on Sparkfun IMC20948 library Advanced Example
        // https://github.com/sparkfun/SparkFun_ICM-20948_ArduinoLibrary/blob/main/examples/Arduino/Example2_Advanced/Example2_Advanced.ino
        icm20948_imu.begin(WIRE_PORT,AD0_VAL);

        // Reset IMU so it is in known stable
        icm20948_imu.swReset();
        delay(200);

        // wake up IMU
        icm20948_imu.sleep(false);
        icm20948_imu.lowPower(false);

        // set continuous sample mode
        icm20948_imu.setSampleMode((ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr), ICM_20948_Sample_Mode_Continuous);

        // Set scale settings
        ICM_20948_fss_t myFSS; // This uses a "Full Scale Settings" structure that can contain values for all configurable sensors
        myFSS.a = gpm8; 
        myFSS.g = dps2000;
        icm20948_imu.setFullScale((ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr), myFSS);

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
        icm20948_imu.setDLPFcfg((ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr), myDLPcfg);
        ICM_20948_Status_e accDLPEnableStat = icm20948_imu.enableDLPF(ICM_20948_Internal_Acc, false);
        ICM_20948_Status_e gyrDLPEnableStat = icm20948_imu.enableDLPF(ICM_20948_Internal_Gyr, false);

        // Enable magnetometer
        icm20948_imu.startupMagnetometer();

        // Enable and setup interrupts
        icm20948_imu.cfgIntActiveLow(true);
        icm20948_imu.cfgIntOpenDrain(false);
        icm20948_imu.cfgIntLatch(false);
        icm20948_imu.intEnableRawDataReady(true);
    }
    else if (mimu_board == MIMUBOARD::mimu_LSM9DS1){
        // [enabled] turns the gyro on or off.
        lsm9ds1_imu.settings.gyro.enabled = true;
        // [scale] sets the full-scale range of the gyroscope.
        // scale can be set to either 245, 500, or 2000 dps
        // Travis West 2022-11-02: I was able to saturate the output with 2000 dps, so this seems like an appropriate setting.
        lsm9ds1_imu.settings.gyro.scale = 2000;
        // [sampleRate] sets the output data rate (ODR) of the gyro
        // sampleRate can be set between 1-6
        // 1 = 14.9    4 = 238
        // 2 = 59.5    5 = 476
        // 3 = 119     6 = 952
        lsm9ds1_imu.settings.gyro.sampleRate = 3; // 59.5Hz ODR
        // [bandwidth] can set the cutoff frequency of the gyro.
        // Allowed values: 0-3. Actual value of cutoff frequency
        // depends on the sample rate. (Datasheet section 7.12)
        lsm9ds1_imu.settings.gyro.bandwidth = 0;
        // [lowPowerEnable] turns low-power mode on or off.
        lsm9ds1_imu.settings.gyro.lowPowerEnable = false; // LP mode off
        // [HPFEnable] enables or disables the high-pass filter
        lsm9ds1_imu.settings.gyro.HPFEnable = true; // HPF disabled
        // [HPFCutoff] sets the HPF cutoff frequency (if enabled)
        // Allowable values are 0-9. Value depends on ODR.
        // (Datasheet section 7.14)
        lsm9ds1_imu.settings.gyro.HPFCutoff = 1; // HPF cutoff = 4Hz
        // [flipX], [flipY], and [flipZ] are booleans that can
        // automatically switch the positive/negative orientation
        // of the three gyro axes.
        lsm9ds1_imu.settings.gyro.flipX = false; // Don't flip X
        lsm9ds1_imu.settings.gyro.flipY = false; // Don't flip Y
        lsm9ds1_imu.settings.gyro.flipZ = false; // Don't flip Z
        // [enabled] turns the acclerometer on or off.
        lsm9ds1_imu.settings.accel.enabled = true; // Enable accelerometer
        // [enableX], [enableY], and [enableZ] can turn on or off
        // select axes of the acclerometer.
        lsm9ds1_imu.settings.accel.enableX = true; // Enable X
        lsm9ds1_imu.settings.accel.enableY = true; // Enable Y
        lsm9ds1_imu.settings.accel.enableZ = true; // Enable Z
        // [scale] sets the full-scale range of the accelerometer.
        // accel scale can be 2, 4, 8, or 16 g's
        // Travis West 2022-11-02: In my experiments I found that the effort required
        // to get much more than 7.5 g of acceleration was significant enough that I
        // was worried about causing damage the internal wiring of the instrument.
        // As such, I think 8 g full scale range or less is appropriate, at least
        // until such time as the mechanical robustness of the instrument is improved.
        lsm9ds1_imu.settings.accel.scale = 8;
        // [sampleRate] sets the output data rate (ODR) of the
        // accelerometer. ONLY APPLICABLE WHEN THE GYROSCOPE IS
        // DISABLED! Otherwise accel sample rate = gyro sample rate.
        // accel sample rate can be 1-6
        // 1 = 10 Hz    4 = 238 Hz
        // 2 = 50 Hz    5 = 476 Hz
        // 3 = 119 Hz   6 = 952 Hz
        lsm9ds1_imu.settings.accel.sampleRate = 3;
        // [bandwidth] sets the anti-aliasing filter bandwidth.
        // Accel cutoff frequency can be any value between -1 - 3. 
        // -1 = bandwidth determined by sample rate
        // 0 = 408 Hz   2 = 105 Hz
        // 1 = 211 Hz   3 = 50 Hz
        lsm9ds1_imu.settings.accel.bandwidth = 0; // BW = 408Hz
        // [highResEnable] enables or disables high resolution 
        // mode for the acclerometer.
        lsm9ds1_imu.settings.accel.highResEnable = false; // Disable HR
        // [highResBandwidth] sets the LP cutoff frequency of
        // the accelerometer if it's in high-res mode.
        // can be any value between 0-3
        // LP cutoff is set to a factor of sample rate
        // 0 = ODR/50    2 = ODR/9
        // 1 = ODR/100   3 = ODR/400
        lsm9ds1_imu.settings.accel.highResBandwidth = 0;  
        // [enabled] turns the magnetometer on or off.
        lsm9ds1_imu.settings.mag.enabled = true; // Enable magnetometer
        // [scale] sets the full-scale range of the magnetometer
        // mag scale can be 4, 8, 12, or 16 Gs
        // Travis West 2022-11-02: Considering that the Earth's magnetic field is
        // generally less than 1 Gs, the lowest setting available is likely best.
        // A higher setting could be used if the sensor were installed next to a
        // strong magnetic field, such as a magnet or speaker, since then the reading
        // would not be saturated and the bias from the magnet could potentially be
        // removed.
        lsm9ds1_imu.settings.mag.scale = 4;
        // [sampleRate] sets the output data rate (ODR) of the
        // magnetometer.
        // mag data rate can be 0-7:
        // 0 = 0.625 Hz  4 = 10 Hz
        // 1 = 1.25 Hz   5 = 20 Hz
        // 2 = 2.5 Hz    6 = 40 Hz
        // 3 = 5 Hz      7 = 80 Hz
        lsm9ds1_imu.settings.mag.sampleRate = 5; // Set OD rate to 20Hz
        // [tempCompensationEnable] enables or disables 
        // temperature compensation of the magnetometer.
        lsm9ds1_imu.settings.mag.tempCompensationEnable = false;
        // [XYPerformance] sets the x and y-axis performance of the
        // magnetometer to either:
        // 0 = Low power mode      2 = high performance
        // 1 = medium performance  3 = ultra-high performance
        lsm9ds1_imu.settings.mag.XYPerformance = 3; // Ultra-high perform.
        // [ZPerformance] does the same thing, but only for the z
        lsm9ds1_imu.settings.mag.ZPerformance = 3; // Ultra-high perform.
        // [lowPowerEnable] enables or disables low power mode in
        // the magnetometer.
        lsm9ds1_imu.settings.mag.lowPowerEnable = false;
        // [operatingMode] sets the operating mode of the
        // magnetometer. operatingMode can be 0-2:
        // 0 = continuous conversion
        // 1 = single-conversion
        // 2 = power down
        lsm9ds1_imu.settings.mag.operatingMode = 0; // Continuous mode

        lsm9ds1_imu.begin();
    }
    return true;
}

void IMU::getData() {
    // Get data from the IMU and save it
    if (mimu_board == MIMUBOARD::mimu_ICM20948) {
        icm20948_imu.getAGMT();

        // read icm20948_imu
        accl[0] = -icm20948_imu.accX() / 1000;
        accl[1] = -icm20948_imu.accY() / 1000;
        accl[2] = icm20948_imu.accZ() / 1000;

        // in degrees/sc
        gyro[0] = icm20948_imu.gyrX();
        gyro[1] = icm20948_imu.gyrY();
        gyro[2] = icm20948_imu.gyrZ();

        // uTesla
        magn[0] = icm20948_imu.magX();
        magn[1] = icm20948_imu.magY();
        magn[2] = icm20948_imu.magZ();
    } else if (mimu_board == MIMUBOARD::mimu_LSM9DS1) {
        if (lsm9ds1_imu.accelAvailable()) {
            lsm9ds1_imu.readAccel();
            // In g's
            accl[0] = lsm9ds1_imu.calcAccel(lsm9ds1_imu.ax);
            accl[1] = lsm9ds1_imu.calcAccel(lsm9ds1_imu.ay);
            accl[2] = lsm9ds1_imu.calcAccel(lsm9ds1_imu.az);
        }
        if (lsm9ds1_imu.gyroAvailable()) {
            lsm9ds1_imu.readGyro();
            // In degrees/sec
            gyro[0] = lsm9ds1_imu.calcGyro(lsm9ds1_imu.gx);
            gyro[1] = lsm9ds1_imu.calcGyro(lsm9ds1_imu.gy);
            gyro[2] = lsm9ds1_imu.calcGyro(lsm9ds1_imu.gz);
        }
        if (lsm9ds1_imu.magAvailable()) {
            lsm9ds1_imu.readMag();
            // In Gauss
            magn[0] = lsm9ds1_imu.calcMag(lsm9ds1_imu.mx);
            magn[1] = lsm9ds1_imu.calcMag(lsm9ds1_imu.my);
            magn[2] = lsm9ds1_imu.calcMag(lsm9ds1_imu.mz);
        }
    }
}

void IMU::sleep() {
    if (mimu_board == MIMUBOARD::mimu_ICM20948) {
        icm20948_imu.sleep(true);
    }
}

void IMU::clearInterrupt() {
    if (mimu_board == MIMUBOARD::mimu_ICM20948) {
        icm20948_imu.clearInterrupts();
    }
}