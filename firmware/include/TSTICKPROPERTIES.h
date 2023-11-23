/******************************************************************** 

Contains definitions useful for the T-Stick 
- General Properties
- Touch Properties
********************************************************************/

/********************************************************************
General properties
    - SDA/SCL pins
    - Button pin
    - fsr pin
    - interrupt pins
    - Enable OSC2
********************************************************************/

/*
Define I2C properties
*/
#define SDA_PIN 12
#define SCL_PIN 11
#define I2CUPDATE_FREQ 400000

/*
Define FSR pin numbers
*/
#define FSR_PIN 3

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
#define IMU_INT_PIN 21
#define FUELGAUE_INT_PIN 17
#define TOUCH_INT 7

/*
Enable second OSC address
*/
#define OSC2

/*
*******************************************************************
Touch properties
    - touch board
    - tstick size (number of touch sensors)
    - default trill settings
*******************************************************************
*/

/*
  Choose the capacitive sensing board
  - Trill
  - Enchanti Custom touch board
*/
#define touch_TRILL
// #define touch_ENCHANTI

/*
Set the amount of capacitive stripes to for the T-Stick, up to 120
*/
#define TSTICK_SIZE 30

/*
    Set default noise threshold
*/
#define TRILL_NOISETHRESHOLD 30
#define ENCHANTI_NOISETHRESHOLD 0