/*
*******************************************************************
Sensor Boards
    - touch board
    - IMU board
    - Fuel guage
*******************************************************************
*/

/*
  Choose the capacitive sensing board
  - Trill
  - IDMIL Capsense Board
  - Enchanti Custom touch board
*/

enum board_TOUCH
{
  touch_TRILL = 0,
  touch_IDMIL = 1,
  touch_ENCHANTI = 2
}
 
/*
 Choose IMU board
  - LSM9DS1
  - ICM20948
*/

enum board_IMU 
{
  imu_ICM20948 = 0,
  imu_LSM9DS1 = 1
}

/*
 Choose fuel gauge
  - MAX17048
  - MAX17055
  - Voltage based
*/

enum board_FUELGAUGE
{
  fg_MAX17055 = 0,
  fg_MAX17048 = 1,
  NONE = 2
}