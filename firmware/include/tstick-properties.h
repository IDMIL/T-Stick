/******************************************************************** 

Contains definitions useful for the T-Stick 
- General Properties
- Task rates
********************************************************************/
// Include Sensor properties
#include "tstick-sensors.h"

/********************************************************************
General properties
    - T-Stick MCU Board
    - Sensor boards
    - SDA/SCL pins
    - Button pin
    - fsr pin
    - interrupt pins
    - Enable OSC2
********************************************************************/
/*
Define the ESP32 Board
*/
//#define board_ENCHANTI_rev1
#define board_ENCHANTI_rev2
// #define board_TINYPICO
// #define board_LOLIND32

/*
Set the amount of capacitive stripes to for the T-Stick, up to 120
*/
#define TSTICK_SIZE 60

/*
Define I2C properties
*/
#ifdef board_ENCHANTI_rev1
#define SDA_PIN 12
#define SCL_PIN 11
#endif
#ifdef board_ENCHANTI_rev2
#define SDA_PIN 21
#define SCL_PIN 14
#endif
#define I2CUPDATE_FREQ 400000

/*
Define FSR pin numbers
*/
#ifdef board_ENCHANTI_rev1
#define FSR_PIN 3
#endif
#ifdef board_ENCHANTI_rev2
#define FSR_PIN 8
#endif

/*
Define Button pin number
*/
#define BUTTON_PIN 9

/*
Define LED pins numbers
*/
#define BLUE_LED 15
#define ORANGE_LED 16
#define NEOPIXEL_PIN 18

/*
Define Interrupt pins for sensors
*/
#ifdef board_ENCHANTI_rev1
#define IMU_INT_PIN 21
#endif 
#ifdef board_ENCHANTI_rev2
#define IMU_INT_PIN 48
#endif
#define FUELGAUE_INT_PIN 17
//#define TOUCH_INT 4

/*
Task Rates
*/
// Timing variables
// Comms
#define LIBMAPPER_UPDATE_RATE 4000 // us (250 Hz)
#define OSC_UPDATE_RATE 20000 // us (50Hz) 

// Sensors
#define TOUCH_UPDATE_RATE 2000 // us (500 Hz)
#define IMU_UPDATE_RATE 2000 // us (500 Hz)
#define ANG_UPDATE_RATE 2000 // us (500 Hz)

// Embedded Gestures
#define GESTURE_UPDATE_RATE 5000 // us (200 Hz)

// Feedback sensors
#define BATTERY_UPDATE_RATE 5000000 // us ( 0.2 Hz)
#define LED_UPDATE_RATE 100000 // us (10 Hz)