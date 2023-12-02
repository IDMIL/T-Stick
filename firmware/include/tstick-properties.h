/******************************************************************** 

Contains definitions useful for the T-Stick 
- General Properties
- Sensor Boards
********************************************************************/

/********************************************************************
General properties
    - T-Stick MCU Board
    - SDA/SCL pins
    - Button pin
    - fsr pin
    - interrupt pins
    - Enable OSC2
********************************************************************/
/*
Define the ESP32 Board
*/
#define board_ENCHANTI
// #define board_TINYPICO
// #define board_LOLIND32

/*
Set the amount of capacitive stripes to for the T-Stick, up to 120
*/
#define TSTICK_SIZE 30

/*
Define I2C properties
*/
#ifdef board_ENCHANTI
#define SDA_PIN 12
#define SCL_PIN 11
#endif
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
#define TOUCH_INT 4

/*
Enable second OSC address
*/
// #define OSC2