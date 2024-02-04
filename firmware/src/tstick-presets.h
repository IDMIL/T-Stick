/******************************************************************** 
Defines presets for the T-Stick
********************************************************************/
// General Properties
#define TSTICK_SIZE 60
#define I2CUPDATE_FREQ 3400000
#define COMMS_UPDATE_RATE 5000 // us (200Hz)
#define TOUCH_UPDATE_RATE 2000 // us (500 Hz)
#define IMU_UPDATE_RATE 5000 // us (500 Hz)
#define ANG_UPDATE_RATE 2000 // us (500 Hz)

// Feedback sensors
#define BATTERY_UPDATE_RATE 5000000 // us ( 0.2 Hz)

//T-Stick Version
//#define tstick_5gw_trill_rev1
//#define tstick_5gw_trill_rev2
//#define tstick_5gw_trill_rev3
#define tstick_5gw_enchanti_rev2
//#define tstick_5gw_enchanti_rev3
//#define tstick_custom

// Specific Properties
#ifdef tstick_5gw_trill_rev1
#define SDA_PIN 12
#define SCL_PIN 11
#define FSR_PIN 3
#define BUTTON_PIN 9
#define NEOPIXEL_PIN 18
#define IMU_INT_PIN 21
#define FUELGAUE_INT_PIN 17
#define imu_ICM20948
#define touch_TRILL

// Initialise sensors
#include "trill-touch.h"
TrillTouch touch;
#endif

#ifdef tstick_5gw_trill_rev2
#define SDA_PIN 21
#define SCL_PIN 14
#define FSR_PIN 8
#define BUTTON_PIN 9
#define BLUE_LED 15
#define ORANGE_LED 16
#define IMU_INT_PIN 48
#define FUELGAUE_INT_PIN 17
#define board_ENCHANTI_rev2
#define imu_ICM20948
#define touch_TRILL
// Initialise sensors
#include "trill-touch.h"
TrillTouch touch;

#include <batt.h>
FUELGAUGE fuelgauge;
fuelgauge_config fg_config = {
    0x36, //i2c_addr
    2000, // capacity (mAh)
    50, // End of charge Current (mA)
    10, // rsense (mOhm)
    3, // empty voltage (V)
    3.88, //recovery voltage (V)
    0, // soc
    0, // rcomp
    0, // tempco
    0, // fullcap
    0, // fullcapnorm
    0, // Charge Cycles
};

#include "imu.h"
IMU imu;

#include "fsr.h"
Fsr fsr;

#include "led.h"
Led led;
#endif

#ifdef tstick_5gw_trill_rev3
#define SDA_PIN 21
#define SCL_PIN 14
#define FSR_PIN 8
#define BUTTON_PIN 9
#define FUELGAUE_INT_PIN 17
#define BLUE_LED 15
#define ORANGE_LED 16
#define board_ENCHANTI_rev3
#define imu_ICM20948
#define touch_TRILL

// Initialise sensors
#include "trill-touch.h"
TrillTouch touch;

#include <batt.h>
FUELGAUGE fuelgauge;
fuelgauge_config fg_config = {
    0x36, //i2c_addr
    2000, // capacity (mAh)
    50, // End of charge Current (mA)
    10, // rsense (mOhm)
    3, // empty voltage (V)
    3.88, //recovery voltage (V)
    0, // soc
    0, // rcomp
    0, // tempco
    0, // fullcap
    0, // fullcapnorm
    0, // Charge Cycles
};

#include "imu.h"
IMU imu;

#include "fsr.h"
Fsr fsr;

#include "led.h"
Led led;
#endif

#ifdef tstick_5gw_enchanti_rev2
#define SDA_PIN 21
#define SCL_PIN 14
#define FSR_PIN 8
#define BUTTON_PIN 9
#define FUELGAUE_INT_PIN 17
#define BLUE_LED 15
#define ORANGE_LED 16
#define board_ENCHANTI_rev2
#define imu_ICM20948
#define touch_ENCHANTI

// Initialise sensors
#include "enchanti-touch.h"
EnchantiTouch touch;

#include <batt.h>
FUELGAUGE fuelgauge;
fuelgauge_config fg_config = {
    0x36, //i2c_addr
    2000, // capacity (mAh)
    50, // End of charge Current (mA)
    10, // rsense (mOhm)
    3, // empty voltage (V)
    3.88, //recovery voltage (V)
    0, // soc
    0, // rcomp
    0, // tempco
    0, // fullcap
    0, // fullcapnorm
    0, // Charge Cycles
};

#include "imu.h"
IMU imu;

#include "fsr.h"
Fsr fsr;

#include "led.h"
Led led;
#endif

#ifdef tstick_5gw_enchanti_rev3
#define SDA_PIN 21
#define SCL_PIN 14
#define FSR_PIN 8
#define BUTTON_PIN 9
#define FUELGAUE_INT_PIN 17
#define BLUE_LED 15
#define ORANGE_LED 16
#define board_ENCHANTI_rev2
#define imu_ICM20948
#define touch_ENCHANTI

// Initialise sensors
#include "Enchanti-touch/enchanti-touch.h"
EnchantiTouch touch;

#include <batt.h>
FUELGAUGE fuelgauge;
fuelgauge_config fg_config = {
    0x36, //i2c_addr
    2000, // capacity (mAh)
    50, // End of charge Current (mA)
    10, // rsense (mOhm)
    3, // empty voltage (V)
    3.88, //recovery voltage (V)
    0, // soc
    0, // rcomp
    0, // tempco
    0, // fullcap
    0, // fullcapnorm
    0, // Charge Cycles
};

#include "imu.h"
IMU imu;

#include "fsr.h"
Fsr fsr;

#include "led.h"
Led led;
#endif

#ifdef tstick_custom
/// Define Board
//#define board_ENCHANTI_rev1
//#define board_ENCHANTI_rev2
//#define board_ENCHANTI_rev3

/// Define Sensors
//#define touch_TRILL
//#define touch_ENCHANTI

//#define imu_ICM20948
//#define imu_LSM9DS1

//#define fg_MAX17055

// Basic libraries
#include "imu.h"
IMU imu;
#include "fsr.h"
Fsr fsr;
#include "led.h"
Led led;

// Include the other sensors
#ifdef touch_TRILL
#include "Trill-touch/trill-touch.h"
TrillTouch touch;
#endif

#ifdef touch_ENCHANTI
#include "Enchanti-touch/enchanti-touch.h"
EnchantiTouch touch;
#endif

#ifdef fg_MAX17055
#include <batt.h>
FUELGAUGE fuelgauge;
fuelgauge_config fg_config = {
    0x36, //i2c_addr
    2000, // capacity (mAh)
    50, // End of charge Current (mA)
    10, // rsense (mOhm)
    3, // empty voltage (V)
    3.88, //recovery voltage (V)
    0, // soc
    0, // rcomp
    0, // tempco
    0, // fullcap
    0, // fullcapnorm
    0, // Charge Cycles
};
#endif
#endif