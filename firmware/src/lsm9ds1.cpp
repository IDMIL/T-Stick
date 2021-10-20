// translation file between library commands for IMU

// Those settings need to be changed in LSM9DS1::init()
// at SparkFunLSM9DS1.cpp file, since LSM9DS1::init() overrites all 
// custom settings. Check bug report in
// https://github.com/sparkfun/SparkFun_LSM9DS1_Arduino_Library/issues/27

#include "lsm9ds1.h"

// global constants for 9 DoF fusion and AHRS (Attitude and Heading Reference System)
#define GyroMeasError PI * (40.0f / 180.0f)     // gyroscope measurement error in rads/s (shown as 3 deg/s)
#define GyroMeasDrift PI * (0.0f / 180.0f)      // gyroscope measurement drift in rad/s/s (shown as 0.0 deg/s/s)
// There is a tradeoff in the beta parameter between accuracy and response speed.
// In the original Madgwick study, beta of 0.041 (corresponding to GyroMeasError of 2.7 degrees/s) was found to give optimal accuracy.
// However, with this value, the LSM9SD0 response time is about 10 seconds to a stable initial quaternion.
// Subsequent changes also require a longish lag time to a stable output, not fast enough for a quadcopter or robot car!
// By increasing beta (GyroMeasError) by about a factor of fifteen, the response time constant is reduced to ~2 sec
// I haven't noticed any reduction in solution accuracy. This is essentially the I coefficient in a PID control sense; 
// the bigger the feedback coefficient, the faster the solution converges, usually at the expense of accuracy. 
// In any case, this is the free parameter in the Madgwick filtering and fusion scheme.
#define beta sqrt(3.0f / 4.0f) * GyroMeasError   // compute beta
#define zeta sqrt(3.0f / 4.0f) * GyroMeasDrift   // compute zeta, the other free parameter in the Madgwick scheme usually set to a small or zero value


LSM9DS1 myIMU; // initialize library class

// Initializing IMU
    bool Imu_LSM9DS1::initIMU() {

        Wire.begin();

        // [enabled] turns the gyro on or off.
            myIMU.settings.gyro.enabled = true;
        // [scale] sets the full-scale range of the gyroscope.
        // scale can be set to either 245, 500, or 2000 dps
            myIMU.settings.gyro.scale = 2000;
        // [sampleRate] sets the output data rate (ODR) of the gyro
        // sampleRate can be set between 1-6
        // 1 = 14.9    4 = 238
        // 2 = 59.5    5 = 476
        // 3 = 119     6 = 952
            myIMU.settings.gyro.sampleRate = 3; // 59.5Hz ODR
        // [bandwidth] can set the cutoff frequency of the gyro.
        // Allowed values: 0-3. Actual value of cutoff frequency
        // depends on the sample rate. (Datasheet section 7.12)
            myIMU.settings.gyro.bandwidth = 0;
        // [lowPowerEnable] turns low-power mode on or off.
            myIMU.settings.gyro.lowPowerEnable = false; // LP mode off
        // [HPFEnable] enables or disables the high-pass filter
            myIMU.settings.gyro.HPFEnable = true; // HPF disabled
        // [HPFCutoff] sets the HPF cutoff frequency (if enabled)
        // Allowable values are 0-9. Value depends on ODR.
        // (Datasheet section 7.14)
            myIMU.settings.gyro.HPFCutoff = 1; // HPF cutoff = 4Hz
        // [flipX], [flipY], and [flipZ] are booleans that can
        // automatically switch the positive/negative orientation
        // of the three gyro axes.
            myIMU.settings.gyro.flipX = false; // Don't flip X
            myIMU.settings.gyro.flipY = false; // Don't flip Y
            myIMU.settings.gyro.flipZ = false; // Don't flip Z

        // [enabled] turns the acclerometer on or off.
            myIMU.settings.accel.enabled = true; // Enable accelerometer
        // [enableX], [enableY], and [enableZ] can turn on or off
        // select axes of the acclerometer.
            myIMU.settings.accel.enableX = true; // Enable X
            myIMU.settings.accel.enableY = true; // Enable Y
            myIMU.settings.accel.enableZ = true; // Enable Z
        // [scale] sets the full-scale range of the accelerometer.
        // accel scale can be 2, 4, 8, or 16 g's
            myIMU.settings.accel.scale = 16;
        // [sampleRate] sets the output data rate (ODR) of the
        // accelerometer. ONLY APPLICABLE WHEN THE GYROSCOPE IS
        // DISABLED! Otherwise accel sample rate = gyro sample rate.
        // accel sample rate can be 1-6
        // 1 = 10 Hz    4 = 238 Hz
        // 2 = 50 Hz    5 = 476 Hz
        // 3 = 119 Hz   6 = 952 Hz
            myIMU.settings.accel.sampleRate = 3;
        // [bandwidth] sets the anti-aliasing filter bandwidth.
        // Accel cutoff frequency can be any value between -1 - 3. 
        // -1 = bandwidth determined by sample rate
        // 0 = 408 Hz   2 = 105 Hz
        // 1 = 211 Hz   3 = 50 Hz
            myIMU.settings.accel.bandwidth = 0; // BW = 408Hz
        // [highResEnable] enables or disables high resolution 
        // mode for the acclerometer.
            myIMU.settings.accel.highResEnable = false; // Disable HR
        // [highResBandwidth] sets the LP cutoff frequency of
        // the accelerometer if it's in high-res mode.
        // can be any value between 0-3
        // LP cutoff is set to a factor of sample rate
        // 0 = ODR/50    2 = ODR/9
        // 1 = ODR/100   3 = ODR/400
            myIMU.settings.accel.highResBandwidth = 0;  

        // [enabled] turns the magnetometer on or off.
            myIMU.settings.mag.enabled = true; // Enable magnetometer
        // [scale] sets the full-scale range of the magnetometer
        // mag scale can be 4, 8, 12, or 16 Gs
            myIMU.settings.mag.scale = 16;
        // [sampleRate] sets the output data rate (ODR) of the
        // magnetometer.
        // mag data rate can be 0-7:
        // 0 = 0.625 Hz  4 = 10 Hz
        // 1 = 1.25 Hz   5 = 20 Hz
        // 2 = 2.5 Hz    6 = 40 Hz
        // 3 = 5 Hz      7 = 80 Hz
            myIMU.settings.mag.sampleRate = 5; // Set OD rate to 20Hz
        // [tempCompensationEnable] enables or disables 
        // temperature compensation of the magnetometer.
            myIMU.settings.mag.tempCompensationEnable = false;
        // [XYPerformance] sets the x and y-axis performance of the
        // magnetometer to either:
        // 0 = Low power mode      2 = high performance
        // 1 = medium performance  3 = ultra-high performance
            myIMU.settings.mag.XYPerformance = 3; // Ultra-high perform.
        // [ZPerformance] does the same thing, but only for the z
            myIMU.settings.mag.ZPerformance = 3; // Ultra-high perform.
        // [lowPowerEnable] enables or disables low power mode in
        // the magnetometer.
            myIMU.settings.mag.lowPowerEnable = false;
        // [operatingMode] sets the operating mode of the
        // magnetometer. operatingMode can be 0-2:
        // 0 = continuous conversion
        // 1 = single-conversion
        // 2 = power down
            myIMU.settings.mag.operatingMode = 0; // Continuous mode

        myIMU.begin();

        return true;
    };

// Read IMU data
    bool Imu_LSM9DS1::dataAvailable() {
        if ( myIMU.gyroAvailable() ) {
            myIMU.readGyro();
        }
        if ( myIMU.accelAvailable() ) {
            myIMU.readAccel();
        }
        if ( myIMU.magAvailable() ) {
            myIMU.readMag();
        }

        Imu_LSM9DS1::accelX = myIMU.calcAccel(myIMU.ax) * 9.80665; // in m/s^2, converted from g's
        Imu_LSM9DS1::accelY = myIMU.calcAccel(myIMU.ay) * 9.80665; // in m/s^2, converted from g's
        Imu_LSM9DS1::accelZ = myIMU.calcAccel(myIMU.az) * 9.80665; // in m/s^2, converted from g's
        Imu_LSM9DS1::gyroX = myIMU.calcGyro(myIMU.gx) * Imu_LSM9DS1::pi / 180; // in radians per second, converted from DPS
        Imu_LSM9DS1::gyroY = myIMU.calcGyro(myIMU.gy) * Imu_LSM9DS1::pi / 180; // in radians per second, converted from DPS
        Imu_LSM9DS1::gyroZ = myIMU.calcGyro(myIMU.gz) * Imu_LSM9DS1::pi / 180; // in radians per second, converted from DPS
        Imu_LSM9DS1::magX = myIMU.calcMag(myIMU.mx) / 10000; // in uTesla, converted from Gauss
        Imu_LSM9DS1::magY = myIMU.calcMag(myIMU.my) / 10000; // in uTesla, converted from Gauss
        Imu_LSM9DS1::magZ = myIMU.calcMag(myIMU.mz) / 10000; // in uTesla, converted from Gauss

        Imu_LSM9DS1::raw_accelX = myIMU.calcAccel(myIMU.ax);
        Imu_LSM9DS1::raw_accelY = myIMU.calcAccel(myIMU.ay);
        Imu_LSM9DS1::raw_accelZ = myIMU.calcAccel(myIMU.az);
        Imu_LSM9DS1::raw_gyroX = myIMU.calcGyro(myIMU.gx);
        Imu_LSM9DS1::raw_gyroY = myIMU.calcGyro(myIMU.gy);
        Imu_LSM9DS1::raw_gyroZ = myIMU.calcGyro(myIMU.gz);
        Imu_LSM9DS1::raw_magX = myIMU.calcMag(myIMU.mx);
        Imu_LSM9DS1::raw_magY = myIMU.calcMag(myIMU.my);
        Imu_LSM9DS1::raw_magZ = myIMU.calcMag(myIMU.mz);

        Imu_LSM9DS1::Now = micros();
        Imu_LSM9DS1::deltat = ((Imu_LSM9DS1::Now - Imu_LSM9DS1::lastUpdate)/1000000.0f); // set integration time by time elapsed since last filter update
        Imu_LSM9DS1::lastUpdate = Imu_LSM9DS1::Now;
        // Sensors x- and y-axes are aligned but magnetometer z-axis (+ down) is opposite to z-axis (+ up) of accelerometer and gyro!
        // This is ok by aircraft orientation standards!  
        // Pass gyro rate as rad/s        
        Imu_LSM9DS1::MadgwickQuaternionUpdate(myIMU.calcAccel(myIMU.ax), myIMU.calcAccel(myIMU.ay), myIMU.calcAccel(myIMU.az), Imu_LSM9DS1::gyroX, Imu_LSM9DS1::gyroY, Imu_LSM9DS1::gyroZ, myIMU.calcMag(myIMU.mx), myIMU.calcMag(myIMU.my), myIMU.calcMag(myIMU.mz));
        Imu_LSM9DS1::taitBryanAngles(Imu_LSM9DS1::q1, Imu_LSM9DS1::q2, Imu_LSM9DS1::q3, Imu_LSM9DS1::q4);
        return 1;
    };

float Imu_LSM9DS1::getAccelX() {
    return Imu_LSM9DS1::accelX;
};

float Imu_LSM9DS1::getAccelY() {
    return Imu_LSM9DS1::accelY;
};

float Imu_LSM9DS1::getAccelZ() {
    return Imu_LSM9DS1::accelZ;
};

float Imu_LSM9DS1::getGyroX() {
    return Imu_LSM9DS1::gyroX;
};

float Imu_LSM9DS1::getGyroY() {
    return Imu_LSM9DS1::gyroY;
};

float Imu_LSM9DS1::getGyroZ() {
    return Imu_LSM9DS1::gyroZ;
};

float Imu_LSM9DS1::getMagX() {
    return Imu_LSM9DS1::magX;
};

float Imu_LSM9DS1::getMagY() {
    return Imu_LSM9DS1::magY;
};

float Imu_LSM9DS1::getMagZ() {
    return Imu_LSM9DS1::magZ;
};

unsigned int Imu_LSM9DS1::getMagAccuracy() {
    return 0;
};

float Imu_LSM9DS1::getQuatI() {
    return Imu_LSM9DS1::q1;
};

float Imu_LSM9DS1::getQuatJ() {
    return Imu_LSM9DS1::q2;
};

float Imu_LSM9DS1::getQuatK() {
    return Imu_LSM9DS1::q3;
};

float Imu_LSM9DS1::getQuatReal() {
    return Imu_LSM9DS1::q4;
};

float Imu_LSM9DS1::getQuatRadianAccuracy() {
    return 0;
};

float Imu_LSM9DS1::getRoll() {
      return Imu_LSM9DS1::roll; // check unit
  };

float Imu_LSM9DS1::getPitch() {
      return Imu_LSM9DS1::pitch; // check unit
  };

float Imu_LSM9DS1::getYaw() {
      return Imu_LSM9DS1::yaw; // check unit
  };

// Implementation of Sebastian Madgwick's "...efficient orientation filter for... inertial/magnetic sensor arrays"
// (see http://www.x-io.co.uk/category/open-source/ for examples and more details)
// which fuses acceleration, rotation rate, and magnetic moments to produce a quaternion-based estimate of absolute
// device orientation -- which can be converted to yaw, pitch, and roll. Useful for stabilizing quadcopters, etc.
// The performance of the orientation filter is at least as good as conventional Kalman-based filtering algorithms
// but is much less computationally intensive---it can be performed on a 3.3 V Pro Mini operating at 8 MHz!
void Imu_LSM9DS1::MadgwickQuaternionUpdate(float ax, float ay, float az, float gx, float gy, float gz, float mx, float my, float mz) {
    // value types -> Accl: g's, Gyro: Rad/sec, and Mag: Gauss
  //float q1 = Imu_LSM9DS1::q1, q2 = Imu_LSM9DS1::q2, q3 = Imu_LSM9DS1::q3, q4 = Imu_LSM9DS1::q4;   // short name local variable for readability

  float norm;
  float hx, hy, _2bx, _2bz;
  float s1, s2, s3, s4;
  float qDot1, qDot2, qDot3, qDot4;

  // Auxiliary variables to avoid repeated arithmetic
  float _2q1mx;
  float _2q1my;
  float _2q1mz;
  float _2q2mx;
  float _4bx;
  float _4bz;
  float _2q1 = 2.0f * q1;
  float _2q2 = 2.0f * q2;
  float _2q3 = 2.0f * q3;
  float _2q4 = 2.0f * q4;
  float _2q1q3 = 2.0f * q1 * q3;
  float _2q3q4 = 2.0f * q3 * q4;
  float q1q1 = q1 * q1;
  float q1q2 = q1 * q2;
  float q1q3 = q1 * q3;
  float q1q4 = q1 * q4;
  float q2q2 = q2 * q2;
  float q2q3 = q2 * q3;
  float q2q4 = q2 * q4;
  float q3q3 = q3 * q3;
  float q3q4 = q3 * q4;
  float q4q4 = q4 * q4;

  // Normalise accelerometer measurement
  norm = sqrt(ax * ax + ay * ay + az * az);
  if (norm == 0.0f) return; // handle NaN
  norm = 1.0f/norm;
  ax *= norm;
  ay *= norm;
  az *= norm;

  // Normalise magnetometer measurement
  norm = sqrt(mx * mx + my * my + mz * mz);
  if (norm == 0.0f) return; // handle NaN
  norm = 1.0f/norm;
  mx *= norm;
  my *= norm;
  mz *= norm;

  // Reference direction of Earth's magnetic field
  _2q1mx = 2.0f * q1 * mx;
  _2q1my = 2.0f * q1 * my;
  _2q1mz = 2.0f * q1 * mz;
  _2q2mx = 2.0f * q2 * mx;
  hx = mx * q1q1 - _2q1my * q4 + _2q1mz * q3 + mx * q2q2 + _2q2 * my * q3 + _2q2 * mz * q4 - mx * q3q3 - mx * q4q4;
  hy = _2q1mx * q4 + my * q1q1 - _2q1mz * q2 + _2q2mx * q3 - my * q2q2 + my * q3q3 + _2q3 * mz * q4 - my * q4q4;
  _2bx = sqrt(hx * hx + hy * hy);
  _2bz = -_2q1mx * q3 + _2q1my * q2 + mz * q1q1 + _2q2mx * q4 - mz * q2q2 + _2q3 * my * q4 - mz * q3q3 + mz * q4q4;
  _4bx = 2.0f * _2bx;
  _4bz = 2.0f * _2bz;

  // Gradient decent algorithm corrective step
  s1 = -_2q3 * (2.0f * q2q4 - _2q1q3 - ax) + _2q2 * (2.0f * q1q2 + _2q3q4 - ay) - _2bz * q3 * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (-_2bx * q4 + _2bz * q2) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + _2bx * q3 * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - mz);  
  s2 = _2q4 * (2.0f * q2q4 - _2q1q3 - ax) + _2q1 * (2.0f * q1q2 + _2q3q4 - ay) - 4.0f * q2 * (1.0f - 2.0f * q2q2 - 2.0f * q3q3 - az) + _2bz * q4 * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (_2bx * q3 + _2bz * q1) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + (_2bx * q4 - _4bz * q2) * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - mz);
  s3 = -_2q1 * (2.0f * q2q4 - _2q1q3 - ax) + _2q4 * (2.0f * q1q2 + _2q3q4 - ay) - 4.0f * q3 * (1.0f - 2.0f * q2q2 - 2.0f * q3q3 - az) + (-_4bx * q3 - _2bz * q1) * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (_2bx * q2 + _2bz * q4) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + (_2bx * q1 - _4bz * q3) * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - mz);
  s4 = _2q2 * (2.0f * q2q4 - _2q1q3 - ax) + _2q3 * (2.0f * q1q2 + _2q3q4 - ay) + (-_4bx * q4 + _2bz * q2) * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (-_2bx * q1 + _2bz * q3) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + _2bx * q2 * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - mz);
  norm = sqrt(s1 * s1 + s2 * s2 + s3 * s3 + s4 * s4);    // normalise step magnitude
  norm = 1.0f/norm;
  s1 *= norm;
  s2 *= norm;
  s3 *= norm;
  s4 *= norm;

  // Compute rate of change of quaternion
  qDot1 = 0.5f * (-q2 * gx - q3 * gy - q4 * gz) - beta * s1;
  qDot2 = 0.5f * (q1 * gx + q3 * gz - q4 * gy) - beta * s2;
  qDot3 = 0.5f * (q1 * gy - q2 * gz + q4 * gx) - beta * s3;
  qDot4 = 0.5f * (q1 * gz + q2 * gy - q3 * gx) - beta * s4;

  // Integrate to yield quaternion
  q1 += qDot1 * Imu_LSM9DS1::deltat;
  q2 += qDot2 * Imu_LSM9DS1::deltat;
  q3 += qDot3 * Imu_LSM9DS1::deltat;
  q4 += qDot4 * Imu_LSM9DS1::deltat;

  norm = sqrt(q1 * q1 + q2 * q2 + q3 * q3 + q4 * q4);    // normalise quaternion
  norm = 1.0f/norm;
  Imu_LSM9DS1::q1 = q1 * norm;
  Imu_LSM9DS1::q2 = q2 * norm;
  Imu_LSM9DS1::q3 = q3 * norm;
  Imu_LSM9DS1::q4 = q4 * norm;
}
  
void Imu_LSM9DS1::taitBryanAngles(float w, float x, float y, float z) {
    Imu_LSM9DS1::roll = atan2(2.0f * (x * y + w * z), w * w + x * x - y * y - z * z);   
    Imu_LSM9DS1::pitch = -asin(2.0f * (x * z - w * y));
    Imu_LSM9DS1::yaw = atan2(2.0f * (w * x + y * z), w * w - x * x - y * y + z * z);
    Imu_LSM9DS1::roll *= 180.0f / PI; 
    Imu_LSM9DS1::roll += Imu_LSM9DS1::declination;
    Imu_LSM9DS1::pitch *= 180.0f / PI;
    Imu_LSM9DS1::yaw *= 180.0f / PI;

    // // Apply temporary offset
    // if (offsetYaw > 0) {
    //     Imu_LSM9DS1::yaw -= offsetYaw;
    //   } else {
    //     Imu_LSM9DS1::yaw += offsetYaw;
    //   }
}