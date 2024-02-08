/******************************************************************** 
Defines presets for the T-Stick
********************************************************************/
//*************************4GW Presets*******************************
//#define tstick_4gw_lolin_capsense
//#define tstick_4gw_lolin_trill
//#define tstick_4gw_tinypico_capsense
//#define tstick_4gw_tinypico_trill

//*************************5GW Presets*******************************
//#define tstick_5gw_trill_rev1
//#define tstick_5gw_trill_rev2
//#define tstick_5gw_trill_rev3
#define tstick_5gw_enchanti_rev2
//#define tstick_5gw_enchanti_rev3

//Custom
//#define tstick_custom


/******************************************************************** 
General Properties for the T-Stick
********************************************************************/
#define TSTICK_SIZE 60
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

    // Boards + Sensors
    #define INDICATOR_LED
    #define imu_LSM9DS1
    #define touch_IDMIL
    #define fg_NONE

    // Initialise sensors
    #include "capsense.h"
    Capsense touch;

    #include "imu.h"
    IMU imu;

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

    // Boards + Sensors
    #define INDICATOR_LED
    #define imu_LSM9DS1
    #define touch_Trill
    #define fg_NONE

    // Initialise sensors
    #include "trill-touch.h"
    TrillTouch touch;

    #include "imu.h"
    IMU imu;

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

    // Boards + Sensors
    #define imu_LSM9DS1
    #define touch_IDMIL
    #define fg_NONE

    // Initialise sensors
    #include "TinyPICO.h"
    TinyPICO tinypico = TinyPICO();

    #include "capsense.h"
    Capsense touch;

    #include "imu.h"
    IMU imu;

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

    // Boards + Sensors
    #define imu_LSM9DS1
    #define touch_Trill
    #define fg_NONE

    // Initialise sensors
    #include "TinyPICO.h"
    TinyPICO tinypico = TinyPICO();

    #include "trill-touch.h"
    TrillTouch touch;

    #include "imu.h"
    IMU imu;

    #include "fsr.h"
    Fsr fsr;

    #include "led.h"
    Led led;
#endif


#ifdef tstick_5gw_trill_rev1
    // Pin definitions
    #define SDA_PIN 12
    #define SCL_PIN 11
    #define FSR_PIN 3
    #define LED_PIN 5
    #define BUTTON_PIN 9
    #define NEOPIXEL_PIN 18
    #define IMU_INT_PIN 21
    #define FUELGAUE_INT_PIN 17

    // Boards + Sensors
    #define imu_ICM20948
    #define touch_TRILL
    #define fg_MAX17055

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

#ifdef tstick_5gw_trill_rev2
    // Pin definitions
    #define SDA_PIN 21
    #define SCL_PIN 14
    #define FSR_PIN 8
    #define BUTTON_PIN 9
    #define LED_PIN 15
    #define ORANGE_LED 16
    #define IMU_INT_PIN 48
    #define FUELGAUE_INT_PIN 17

    // Boards + Sensors
    #define INDICATOR_LED
    #define board_ENCHANTI_rev2
    #define imu_ICM20948
    #define touch_TRILL
    #define fg_MAX17055

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
    // Pin definitions
    #define SDA_PIN 21
    #define SCL_PIN 14
    #define FSR_PIN 8
    #define BUTTON_PIN 9
    #define FUELGAUE_INT_PIN 17
    #define LED_PIN 15
    #define ORANGE_LED 16

    // Boards + Sensors
    #define INDICATOR_LED
    #define board_ENCHANTI_rev3
    #define imu_ICM20948
    #define touch_TRILL
    #define fg_MAX17055

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
    // Pin definitions
    #define SDA_PIN 21
    #define SCL_PIN 14
    #define FSR_PIN 8
    #define BUTTON_PIN 9
    #define FUELGAUE_INT_PIN 17
    #define LED_PIN 15
    #define ORANGE_LED 16

    // Boards + Sensors
    #define INDICATOR_LED
    #define board_ENCHANTI_rev2
    #define imu_ICM20948
    #define touch_ENCHANTI
    #define fg_MAX17055

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
    // Pin definitions
    #define SDA_PIN 21
    #define SCL_PIN 14
    #define FSR_PIN 8
    #define BUTTON_PIN 9
    #define FUELGAUE_INT_PIN 17
    #define LED_PIN 15
    #define ORANGE_LED 16

    // Boards + Sensors
    #define INDICATOR_LED
    #define board_ENCHANTI_rev2
    #define imu_ICM20948
    #define touch_ENCHANTI
    #define fg_MAX17055
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

    //#define imu_ICM20948
    //#define imu_LSM9DS1

    //#define fg_MAX17055
    //#define fg_NONE

    // #define INDICATOR_LED // use with LED_PIN if using a discrete LED and LED PWM library

    // Basic libraries
    #include "imu.h"
    IMU imu;
    #include "fsr.h"
    Fsr fsr;
    #include "led.h"
    Led led;

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