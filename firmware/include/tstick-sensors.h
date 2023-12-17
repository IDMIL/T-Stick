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

/*
Define the Touch Board
*/
#define touch_TRILL
//#define touch_IDMIL
//#define touch_ENCHANTI

/*
Define the IMU
*/
#define imu_ICM20948
//#define imu_LSM9DS1 

/*
Define the fuel gauge
  - MAX17048
  - MAX17055
  - Voltage based
*/
#define fg_MAX17055
//#define fg_MAX17048 
//#define fg_NONE