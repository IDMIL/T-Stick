/******************************************************************** 
Defines presets for the T-Stick
********************************************************************/
//*************************4GW Presets*******************************
//#define tstick_4gw_lolin_capsense
//#define tstick_4gw_lolin_trill
//#define tstick_4gw_tinypico_capsense
//#define tstick_4gw_tinypico_trill

//*************************5GW Presets*******************************
//#define tstick_5gw_trill_beta
//#define tstick_5gw_enchanti_beta
//#define tstick_5gw_trill_main
//#define tstick_5gw_enchanti_main

//Custom
//#define tstick_custom


/******************************************************************** 
General Properties for the T-Stick
********************************************************************/
#include "imu-cal.h"
//#define TSTICK_SIZE 60
#define I2CUPDATE_FREQ 400000 // Note that the I2C frequency is capped by Wire at 1MHz

// Feedback sensors
#define BATTERY_UPDATE_RATE 1000000 // us ( 1 Hz)

// Specific Properties
#ifdef tstick_4gw_lolin_capsense
    // Pin definitions
    #define SDA_PIN 22
    #define SCL_PIN 21
    #define FSR_PIN 33
    #define LED_PIN 5
    #define BUTTON_PIN 15
    #define SLEEP_PIN GPIO_NUM_15

    // Boards + Sensors
    #define INDICATOR_LED
    #define imu_LSM9DS1
    #define touch_IDMIL
    #define fg_NONE

    // Initialise sensors
    #include <driver/rtc_io.h> // needed for sleep
    #include "button.h"

    Button button;

    #include "capsense.h"
    #define TOUCH_MAX 1
    Capsense touch;
    touch_config tstick_touchconfig = {
        default_config.touchdevice,
        default_config.touchsize,
        default_config.touch_threshold,
        default_config.touch_mode,
        default_config.comm_mode,
    };

    #include "imu.h"
    IMU imu;
    #define TSTICK_IMU MIMUBOARD::mimu_LSM9DS1

    #include "fsr.h"
    Fsr fsr;

    #include "led.h"
    Led led;
#endif

#ifdef tstick_4gw_lolin_trill
    // Pin definitions
    #define SDA_PIN 22
    #define SCL_PIN 21
    #define FSR_PIN 33
    #define LED_PIN 5
    #define BATTERY_PIN 35 // read battery voltage
    #define BUTTON_PIN 15
    #define SLEEP_PIN GPIO_NUM_15

    // Boards + Sensors
    #define INDICATOR_LED
    #define imu_LSM9DS1
    #define touch_Trill
    #define fg_NONE

    // Initialise sensors
    #include <driver/rtc_io.h> // needed for sleep
    #include "button.h"

    Button button;

    #include "trill-touch.h"
    #define TOUCH_MAX 511
    TrillTouch touch;
    touch_config tstick_touchconfig = {
        default_config.touchdevice,
        default_config.touchsize,
        default_config.touch_threshold,
        default_config.touch_mode,
        default_config.comm_mode,
    };

    #include "imu.h"
    IMU imu;
    #define TSTICK_IMU MIMUBOARD::mimu_LSM9DS1

    #include "fsr.h"
    Fsr fsr;

    #include "led.h"
    Led led;
#endif

#ifdef tstick_4gw_tinypico_capsense
    // Pin definitions
    #define SDA_PIN 22
    #define SCL_PIN 21
    #define FSR_PIN 33
    #define LED_PIN 5
    #define BUTTON_PIN 15
    #define SLEEP_PIN GPIO_NUM_15

    // Boards + Sensors
    #define imu_LSM9DS1
    #define touch_IDMIL
    #define fg_NONE

    // Initialise sensors
    #include <driver/rtc_io.h> // needed for sleep
    #include "button.h"

    Button button;

    #include "TinyPICO.h"
    TinyPICO tinypico = TinyPICO();

    #include "capsense.h"
    #define TOUCH_MAX 1
    Capsense touch;
    touch_config tstick_touchconfig = {
        default_config.touchdevice,
        default_config.touchsize,
        default_config.touch_threshold,
        default_config.touch_mode,
        default_config.comm_mode,
    };

    #include "imu.h"
    IMU imu;
    #define TSTICK_IMU MIMUBOARD::mimu_LSM9DS1

    #include "fsr.h"
    Fsr fsr;

    #include "led.h"
    Led led;
#endif

#ifdef tstick_4gw_tinypico_trill
    // Pin definitions
    #define SDA_PIN 22
    #define SCL_PIN 21
    #define FSR_PIN 33
    #define LED_PIN 5
    #define BATTERY_PIN 35 // read battery voltage
    #define BUTTON_PIN 15
    #define SLEEP_PIN GPIO_NUM_15

    // Boards + Sensors
    #define imu_LSM9DS1
    #define touch_Trill
    #define fg_NONE

    // Initialise sensors
    #include <driver/rtc_io.h> // needed for sleep
    #include "button.h"

    Button button;

    #include "TinyPICO.h"
    TinyPICO tinypico = TinyPICO();

    #include "trill-touch.h"
    #define TOUCH_MAX 511
    TrillTouch touch;
    touch_config tstick_touchconfig = {
        default_config.touchdevice,
        default_config.touchsize,
        default_config.touch_threshold,
        default_config.touch_mode,
        default_config.comm_mode,
    };

    #include "imu.h"
    IMU imu;
    #define TSTICK_IMU MIMUBOARD::mimu_LSM9DS1

    #include "fsr.h"
    Fsr fsr;

    #include "led.h"
    Led led;
#endif


#ifdef tstick_5gw_trill_beta
    // Pin definitions
    #define SDA_PIN 12
    #define SCL_PIN 11
    #define FSR_PIN 3
    #define LED_PIN 5
    #define BUTTON_PIN 9
    #define NEOPIXEL_PIN 18
    #define IMU_INT_PIN 21
    #define FUELGAUE_INT_PIN 17
    #define SLEEP_PIN GPIO_NUM_9

    // Boards + Sensors
    #define imu_ICM20948
    #define touch_TRILL
    #define fg_MAX17055

    // Initialise sensors
    #include <driver/rtc_io.h> // needed for sleep
    #include "button.h"

    Button button;

    #include "trill-touch.h"
    #define TOUCH_MAX 511
    TrillTouch touch;
    touch_config tstick_touchconfig = {
        default_config.touchdevice,
        default_config.touchsize,
        default_config.touch_threshold,
        default_config.touch_mode,
        default_config.comm_mode,
    };

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
    #define TSTICK_IMU MIMUBOARD::mimu_ICM20948

    #include "fsr.h"
    Fsr fsr;

    #include "led.h"
    Led led;
#endif

#ifdef tstick_5gw_trill_main
    // Pin definitions
    #define SDA_PIN 21
    #define SCL_PIN 14
    #define FSR_PIN 8
    #define BUTTON_PIN 9
    #define LED_PIN 15
    #define ORANGE_LED 16
    #define IMU_INT_PIN 48
    #define FUELGAUE_INT_PIN 17
    #define SLEEP_PIN GPIO_NUM_9

    // Boards + Sensors
    #define INDICATOR_LED
    #define board_ENCHANTI_rev2
    #define imu_ICM20948
    #define touch_TRILL
    #define fg_MAX17055

    // Initialise sensors
    #include <driver/rtc_io.h> // needed for sleep
    #include "button.h"

    Button button;

    #include "trill-touch.h"
    #define TOUCH_MAX 511
    TrillTouch touch;
    touch_config tstick_touchconfig = {
        default_config.touchdevice,
        default_config.touchsize,
        default_config.touch_threshold,
        default_config.touch_mode,
        default_config.comm_mode,
    };

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
    #define TSTICK_IMU MIMUBOARD::mimu_ICM20948

    #include "fsr.h"
    Fsr fsr;

    #include "led.h"
    Led led;
#endif

#ifdef tstick_5gw_enchanti_main
    // Pin definitions
    #define SDA_PIN 21
    #define SCL_PIN 14
    #define FSR_PIN 8
    #define LDO_PIN 39
    #define BUTTON_PIN 9
    #define FUELGAUE_INT_PIN 17
    #define IMU_INT_PIN 48
    #define LED_PIN 15
    #define ORANGE_LED 16
    #define SLEEP_PIN GPIO_NUM_9

    // Boards + Sensors
    #define LDO2
    #define INDICATOR_LED
    #define board_ENCHANTI_rev2
    #define imu_ICM20948
    #define touch_ENCHANTI
    #define fg_MAX17055

    // Initialise sensors
    #include <driver/rtc_io.h> // needed for sleep
    #include "button.h"

    Button button;

    #include "enchanti-touch.h"
    #define TOUCH_MAX 4095
    EnchantiTouch touch;
    touch_config tstick_touchconfig = {
        default_config.touchdevice,
        default_config.touchsize,
        default_config.touch_threshold,
        default_config.touch_mode,
        default_config.comm_mode,
    };

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
    #define TSTICK_IMU MIMUBOARD::mimu_ICM20948

    #include "fsr.h"
    Fsr fsr;

    #include "led.h"
    Led led;
#endif

#ifdef tstick_custom
    // Basic libraries
    #include "imu.h"
    IMU imu;
    #include "fsr.h"
    Fsr fsr;
    #include "led.h"
    Led led;

    // Define pins
    // #define SDA_PIN 21
    // #define SCL_PIN 14
    // #define FSR_PIN 8
    // #define BUTTON_PIN 9
    // #define FUELGAUE_INT_PIN 17
    // #define LED_PIN 15
    // #define ORANGE_LED 16

    /// Define Board
    //#define board_ENCHANTI_rev1
    //#define board_ENCHANTI_rev2
    //#define board_ENCHANTI_rev3

    /// Define Sensors
    //#define touch_TRILL
    //#define touch_ENCHANTI
    //#define touch_IDMIL

    //#define TSTICK_IMU MIMUBOARD::mimu_ICM20948
    //#define TSTICK_IMU MIMUBOARD::mimu_LSM9DS1

    //#define fg_MAX17055
    //#define fg_NONE

    // #define INDICATOR_LED // use with LED_PIN if using a discrete LED and LED PWM library

    // Include the other sensors
    #ifdef touch_IDMIL
        #include "capsense.h"
        Capsense touch;
    #endif

    #ifdef touch_TRILL
        #include "trill-touch.h"
        TrillTouch touch;
    #endif

    #ifdef touch_ENCHANTI
        #include "enchanti-touch.h"
        EnchantiTouch touch;
    #endif

    // Define the config for your touch board
    #define TOUCH_MAX 4095 // define maximum touch value
    touch_config tstick_touchconfig = {
        -1, // default touch device
        -1, // default touch size
        -1, // noise threshold
        -1, // touch processing mode
        -1, // comm mode
    };


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
