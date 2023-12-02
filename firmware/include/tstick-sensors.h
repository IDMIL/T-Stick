/******************************************************************** 
Includes sensor properties
- MIMU
- Fuel gauge
- Touch Sensor
********************************************************************/

#include "tstick-properties.h"
#include "touch.h"
#include "button.h"
#include "led.h"
#include "fsr.h"
#include "batt.h"
#include "imu.h"


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
#define touch_TRILL
//#define touch_IDMIL
//#define touch_ENCHANTI

/*
 Choose IMU board
  - LSM9DS1
  - ICM20948
*/
#define ICM20948
//#define LSM9DS1

/*
 Choose fuel gauge
  - MAX17048
  - MAX17055
  - Voltage based
*/
#define MAX17055
//#define MAX17048
//#define VOLTAGEBASED

/*
*******************************************************************
Touch Sensor
    - touch board
    - default settings
*******************************************************************
*/
#define TRILL_TYPE 3 // 3 for Trill craft, 6 for trill flex
#define NOISETHRESHOLD 0