//****************************************************************************//
// T-Stick - sopranino/soprano firmware                                       //
// SAT/Metalab                                                                //
// Input Devices and Music Interaction Laboratory (IDMIL), McGill University  //
// Edu Meneses (2022) - https://www.edumeneses.com                            //
//****************************************************************************//

/* Created using the Puara template: https://github.com/Puara/puara-module-template 
 * The template contains a fully commented version for the commonly used commands 
 */


unsigned int firmware_version = 231031;

/*
Include T-Stick properties
- Go to include/TSTICKPROPERTIES.h to edit properties for your T-Stick
*/ 
#include "tstick-properties.h"
#include "tstick-sensors.h"

// Sensor libraries
#include "touch.h"

#include "Arduino.h"
// For JTAG monitor
#include "USB.h"

// For task scheduling
#include "TaskScheduler.h"

// For disabling power saving
#include "esp_wifi.h"

#include <puara.h>
#include <puara_gestures.h>
#include <mapper.h>

#include <deque>
#include <cmath>
#include <algorithm>

// initializing libmapper, puara, puara-gestures, and liblo client
mpr_dev lm_dev = 0;
Puara puara;
PuaraGestures gestures;
lo_address osc1;
lo_address osc2;
std::string baseNamespace = "/";
std::string oscNamespace;

///////////////////////////
// Calibration Structure //
///////////////////////////
// struct calibrationParameters {
//     // Accelerometer Parameters
//     float accel_zerog[3];

//     // Gyroscope Parameters
//     float gyro_zerorate[3];
    
//     // Magnetometer Parameters
//     float sx[3];
//     float sy[3];
//     float sz[3];
//     float h[3];
// };
float accel_zerog[3];
float gyro_zerorate[3];
float sx[3];
float sy[3];
float sz[3];
float h[3];

calibrationParameters imuParams;


/////////////////////
// Pin definitions //
/////////////////////

struct TSTICK_Pin {
    int fsr;
    int button;
};

TSTICK_Pin pin{ FSR_PIN, BUTTON_PIN };

////////////////////////////////
// Sensor Events //
////////////////////////////////

struct Event {
    bool shake = false;
    bool jab = false;
    bool count = false;
    bool tap = false;
    bool dtap = false;
    bool ttap = false;
    bool brush = false;
    bool rub = false;
    bool mimu = false;
    bool touchReady = false;
    bool battery = false;
    bool current = false;
    bool voltage = false;
} event;


//////////////////////////////////
// Battery struct and functions //
//////////////////////////////////
#include <batt.h>
  
struct BatteryData {
    unsigned int percentage = 0;
    float voltage = 0;
    unsigned int current = 0;
    float TTE = 0;
    bool status = false; // is there a battery
    unsigned int rsense = 0;
    unsigned int capacity = 0;
    unsigned int designcap = 0;
    float value;
    unsigned long timer = 0;
    int interval = 1000; // in ms (1/f)
} battery;

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

///////////////////////////////////
// Include button function files //
///////////////////////////////////
#include <driver/rtc_io.h> // needed for sleep
#include "button.h"

Button button;

void buttton_isr() {
    button.readButton();
}

////////////////////////////////
// Include FSR function files //
////////////////////////////////

#include "fsr.h"

Fsr fsr;

////////////////////////////////
// Include IMU function files //
////////////////////////////////

#include "imu.h"
IMU imu;

//////////////////////////////////////////////
// Include Touch stuff                      //
//////////////////////////////////////////////

// Touch arrays 
int mergedtouch[TSTICK_SIZE]; 
int mergeddiscretetouch[TSTICK_SIZE]; 
int mergednormalisedtouch[TSTICK_SIZE]; 

////////////////////////////////
// Include LED function files //
////////////////////////////////
#include "led.h"

Led led;

struct Led_variables {
    int ledValue = 0;
    uint8_t color = 0;
} led_var;

//////////////////////
// Liblo OSC server //
//////////////////////

void error(int num, const char *msg, const char *path) {
    printf("Liblo server error %d in path %s: %s\n", num, path, msg);
    fflush(stdout);
}
lo_server_thread osc_server;

int generic_handler(const char *path, const char *types, lo_arg ** argv,
                    int argc, lo_message data, void *user_data) {
    for (int i = 0; i < argc; i++) {
        printf("arg %d '%c' ", i, types[i]);
        lo_arg_pp((lo_type)types[i], argv[i]);
        printf("\n");
    }
    printf("\n");
    fflush(stdout);

    return 1;
}

////////////////////////////////
// sensors and libmapper data //
////////////////////////////////

struct Lm {
    mpr_sig fsr = 0;
    int fsrMax = 4095;
    int fsrMin = 0;
    mpr_sig squeeze = 0;
    float squeezeMax = 1.0f;
    float squeezeMin = 0.0f;
    mpr_sig accel = 0;
    float accelMax[3] = {50, 50, 50};
    float accelMin[3] = {-50, -50, -50};
    mpr_sig gyro = 0;
    float gyroMax[3] = {25, 25, 25};
    float gyroMin[3] = {-25, -25, -25};
    mpr_sig magn = 0;
    float magnMax[3] = {25, 25, 25};
    float magnMin[3] = {-25, -25, -25};
    mpr_sig quat = 0;
    float quatMax[4] = {1, 1, 1, 1};
    float quatMin[4] = {-1, -1, -1, -1};
    mpr_sig ypr = 0;
    float yprMax[3] = {M_PI, M_PI_2, M_PI};
    float yprMin[3] = {-M_PI, -M_PI_2, -M_PI};
    mpr_sig shake = 0;
    float shakeMax[3] = {100, 100, 100};
    float shakeMin[3] = {0, 0, 0};
    mpr_sig jab = 0;
    float jabMax[3] = {50, 50, 50};
    float jabMin[3] = {-50, -50, -50};
    mpr_sig brush = 0;
    mpr_sig multibrush = 0;
    float brushMax[4] = {50, 50, 50, 50};
    float brushMin[4] = {-50, -50, -50, -50};
    mpr_sig rub = 0;
    mpr_sig multirub = 0;
    float rubMax[4] = {5, 5, 5, 5};
    float rubMin[4] = {0, 0, 0, 0};
    mpr_sig rawtouch = 0;
    mpr_sig disctouch = 0;
    int touchMax[TSTICK_SIZE]; // Initialized in setup()
    int touchMin[TSTICK_SIZE];
    mpr_sig count = 0;
    int countMax = 100;
    int countMin = 0;
    mpr_sig tap = 0;
    mpr_sig ttap = 0;
    mpr_sig dtap = 0;
    int tapMax = 1;
    int tapMin = 0;
    mpr_sig soc = 0;
    int batSOCMax = 100;
    int batSOCMin = 0;
    mpr_sig batvolt = 0;
    float batVoltMax = 4.2f;
    float batVoltMin = 0.0f;
} lm;

struct Sensors {
    float accl [3];
    float gyro [3];
    float magn [3];
    float quat [4];
    float ypr [3];
    float shake [3];
    float jab [3];
    float brush;
    float rub;
    float multibrush [4];
    float multirub [4];
    int count;
    int tap;
    int dtap;
    int ttap;
    int fsr;
    float squeeze;
    int battery;
    int current;
    float voltage;
} sensors;

/////////////////
// Setup Tasks //
/////////////////
Scheduler runnerComms;
Scheduler runnerSensors;

// task callbacks
// Comms tasks
void pollLibmapper();
void updateLibmapper();
void updateOSC1();
void updateOSC2();

// Sensor callbacks
void readIMU();
void readTouch();
void readAnalog();
void readBattery();
void changeLED();
void updateGestures();

// Setup sensor tasks (task rates defined in tstick-properties.h)
Task updateIMU (TASK_IMMEDIATE, TASK_ONCE, &readIMU, &runnerSensors, false);
// Task updateTOUCH (TOUCH_UPDATE_RATE, TASK_FOREVER, &readTouch, &runnerSensors, false);
Task updateANALOG (ANG_UPDATE_RATE, TASK_FOREVER, &readAnalog, &runnerSensors, false);
Task updateGesture (GESTURE_UPDATE_RATE, TASK_FOREVER, &updateGestures, &runnerSensors, false);
Task updateBattery (BATTERY_UPDATE_RATE, TASK_FOREVER, &readBattery, &runnerSensors, false);
Task updateLED (LED_UPDATE_RATE, TASK_FOREVER, &changeLED, &runnerSensors, false);

// Setup comms tasks
Task libmapperPoll (LIBMAPPER_POLL_RATE, TASK_FOREVER, &pollLibmapper, &runnerComms, true);
Task libmapperUpdate (LIBMAPPER_UPDATE_RATE, TASK_FOREVER, &updateLibmapper, &runnerComms, true);
Task OSCupdate1 (OSC_UPDATE_RATE, TASK_FOREVER, &updateOSC1, &runnerComms, true);
Task OSCupdate2 (OSC_UPDATE_RATE, TASK_FOREVER, &updateOSC2, &runnerComms, false);

// Define callbacks
///// Comms
void pollLibmapper() {
    mpr_dev_poll(lm_dev, 0);
}
void updateLibmapper() {
    mpr_sig_set_value(lm.fsr, 0, 1, MPR_INT32, &sensors.fsr);
    mpr_sig_set_value(lm.accel, 0, 3, MPR_FLT, &sensors.accl);
    mpr_sig_set_value(lm.gyro, 0, 3, MPR_FLT, &sensors.gyro);
    mpr_sig_set_value(lm.magn, 0, 3, MPR_FLT, &sensors.magn);
    mpr_sig_set_value(lm.quat, 0, 4, MPR_FLT, &sensors.quat);
    mpr_sig_set_value(lm.ypr, 0, 3, MPR_FLT, &sensors.ypr);
    mpr_sig_set_value(lm.squeeze, 0, 1, MPR_FLT, &sensors.squeeze);
    mpr_sig_set_value(lm.shake, 0, 3, MPR_FLT, &sensors.shake);
    mpr_sig_set_value(lm.jab, 0, 3, MPR_FLT, &sensors.jab);
    mpr_sig_set_value(lm.rub, 0, 1, MPR_FLT, &sensors.rub);
    mpr_sig_set_value(lm.brush, 0, 1, MPR_FLT, &sensors.brush);
    mpr_sig_set_value(lm.multirub, 0, 4, MPR_FLT, &sensors.multirub);
    mpr_sig_set_value(lm.multibrush, 0, 4, MPR_FLT, &sensors.multibrush);
    mpr_sig_set_value(lm.count, 0, 1, MPR_INT32, &sensors.count);
    mpr_sig_set_value(lm.tap, 0, 1, MPR_INT32, &sensors.tap);
    mpr_sig_set_value(lm.ttap, 0, 1, MPR_INT32, &sensors.dtap);
    mpr_sig_set_value(lm.dtap, 0, 1, MPR_INT32, &sensors.ttap);
    mpr_sig_set_value(lm.soc, 0, 1, MPR_FLT, &sensors.battery);
    mpr_sig_set_value(lm.batvolt, 0, 1, MPR_FLT, &battery.value);
    mpr_sig_set_value(lm.rawtouch, 0, TSTICK_SIZE, MPR_INT32, &mergedtouch);
    mpr_sig_set_value(lm.disctouch, 0, TSTICK_SIZE, MPR_INT32, &mergeddiscretetouch);
}
void updateOSC1() {
    if (puara.IP1_ready()) {
        // Continuously send FSR data
        oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "raw/fsr");
        lo_send(osc1, oscNamespace.c_str(), "i", sensors.fsr);
        oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/squeeze");
        lo_send(osc1, oscNamespace.c_str(), "f", sensors.squeeze);

        // Send discrete messages
        if (event.touchReady) {
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "raw/capsense");
            if (TSTICK_SIZE == 30) {
                lo_send(osc1, oscNamespace.c_str(), "iiiiiiiiiiiiiiiiiiiiiiiiiiiiii", mergedtouch[0], mergedtouch[1],mergedtouch[2],
                mergedtouch[3],mergedtouch[4],mergedtouch[5], mergedtouch[6], mergedtouch[7], mergedtouch[8],
                mergedtouch[9], mergedtouch[10], mergedtouch[11], mergedtouch[12], mergedtouch[13], mergedtouch[14], mergedtouch[15], mergedtouch[16],mergedtouch[17],
                mergedtouch[18],mergedtouch[19],mergedtouch[20], mergedtouch[21], mergedtouch[22], mergedtouch[23],
                mergedtouch[24], mergedtouch[25], mergedtouch[26], mergedtouch[27], mergedtouch[28], mergedtouch[29]);
                oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/touch/normalised");
                lo_send(osc1, oscNamespace.c_str(), "iiiiiiiiiiiiiiiiiiiiiiiiiiiiii", mergednormalisedtouch[0], mergednormalisedtouch[1],mergednormalisedtouch[2],
                mergednormalisedtouch[3],mergednormalisedtouch[4],mergednormalisedtouch[5], mergednormalisedtouch[6], mergednormalisedtouch[7], mergednormalisedtouch[8],
                mergednormalisedtouch[9], mergednormalisedtouch[10], mergednormalisedtouch[11], mergednormalisedtouch[12], mergednormalisedtouch[13], mergednormalisedtouch[14], mergednormalisedtouch[15], mergednormalisedtouch[16],mergednormalisedtouch[17],
                mergednormalisedtouch[18],mergednormalisedtouch[19],mergednormalisedtouch[20], mergednormalisedtouch[21], mergednormalisedtouch[22], mergednormalisedtouch[23],
                mergednormalisedtouch[24], mergednormalisedtouch[25], mergednormalisedtouch[26], mergednormalisedtouch[27], mergednormalisedtouch[28], mergednormalisedtouch[29]);
                oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/touch/discrete");
                lo_send(osc1, oscNamespace.c_str(), "iiiiiiiiiiiiiiiiiiiiiiiiiiiiii", mergeddiscretetouch[0], mergeddiscretetouch[1],mergeddiscretetouch[2],
                mergeddiscretetouch[3],mergeddiscretetouch[4],mergeddiscretetouch[5], mergeddiscretetouch[6], mergeddiscretetouch[7], mergeddiscretetouch[8],
                mergeddiscretetouch[9], mergeddiscretetouch[10], mergeddiscretetouch[11], mergeddiscretetouch[12], mergeddiscretetouch[13], mergeddiscretetouch[14], mergeddiscretetouch[15], mergeddiscretetouch[16],mergeddiscretetouch[17],
                mergeddiscretetouch[18],mergeddiscretetouch[19],mergeddiscretetouch[20], mergeddiscretetouch[21], mergeddiscretetouch[22], mergeddiscretetouch[23],
                mergeddiscretetouch[24], mergeddiscretetouch[25], mergeddiscretetouch[26], mergeddiscretetouch[27], mergeddiscretetouch[28], mergeddiscretetouch[29]);
            } else if (TSTICK_SIZE == 60) {
                // Send data from the first board
                lo_send(osc1, oscNamespace.c_str(), "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii", mergedtouch[0], mergedtouch[1],mergedtouch[2],
                mergedtouch[3],mergedtouch[4],mergedtouch[5], mergedtouch[6], mergedtouch[7], mergedtouch[8],
                mergedtouch[9], mergedtouch[10], mergedtouch[11], mergedtouch[12], mergedtouch[13], mergedtouch[14], mergedtouch[15], mergedtouch[16],mergedtouch[17],
                mergedtouch[18],mergedtouch[19],mergedtouch[20], mergedtouch[21], mergedtouch[22], mergedtouch[23],
                mergedtouch[24], mergedtouch[25], mergedtouch[26], mergedtouch[27], mergedtouch[28], mergedtouch[29], mergedtouch[30], mergedtouch[31], 
                mergedtouch[32], mergedtouch[33], mergedtouch[34], mergedtouch[35], mergedtouch[36], mergedtouch[37], mergedtouch[38], mergedtouch[39], 
                mergedtouch[40], mergedtouch[41], mergedtouch[42], mergedtouch[43], mergedtouch[44],mergedtouch[45],mergedtouch[46], mergedtouch[47], 
                mergedtouch[48], mergedtouch[49], mergedtouch[50],mergedtouch[51], mergedtouch[52],mergedtouch[53],
                mergedtouch[54], mergedtouch[55], mergedtouch[56], mergedtouch[57], mergedtouch[58], mergedtouch[59]);
                oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/touch/normalised");
                lo_send(osc1, oscNamespace.c_str(), "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii", mergednormalisedtouch[0], mergednormalisedtouch[1],mergednormalisedtouch[2],
                mergednormalisedtouch[3],mergednormalisedtouch[4],mergednormalisedtouch[5], mergednormalisedtouch[6], mergednormalisedtouch[7], mergednormalisedtouch[8],
                mergednormalisedtouch[9], mergednormalisedtouch[10], mergednormalisedtouch[11], mergednormalisedtouch[12], mergednormalisedtouch[13], mergednormalisedtouch[14], mergednormalisedtouch[15], mergednormalisedtouch[16],mergednormalisedtouch[17],
                mergednormalisedtouch[18],mergednormalisedtouch[19],mergednormalisedtouch[20], mergednormalisedtouch[21], mergednormalisedtouch[22], mergednormalisedtouch[23],
                mergednormalisedtouch[24], mergednormalisedtouch[25], mergednormalisedtouch[26], mergednormalisedtouch[27], mergednormalisedtouch[28], mergednormalisedtouch[29], mergednormalisedtouch[30], mergednormalisedtouch[31], 
                mergednormalisedtouch[32], mergednormalisedtouch[33], mergednormalisedtouch[34], mergednormalisedtouch[35], mergednormalisedtouch[36], mergednormalisedtouch[37], mergednormalisedtouch[38], mergednormalisedtouch[39], 
                mergednormalisedtouch[40], mergednormalisedtouch[41], mergednormalisedtouch[42], mergednormalisedtouch[43], mergednormalisedtouch[44],mergednormalisedtouch[45],mergednormalisedtouch[46], mergednormalisedtouch[47], 
                mergednormalisedtouch[48], mergednormalisedtouch[49], mergednormalisedtouch[50],mergednormalisedtouch[51], mergednormalisedtouch[52],mergednormalisedtouch[53],
                mergednormalisedtouch[54], mergednormalisedtouch[55], mergednormalisedtouch[56], mergednormalisedtouch[57], mergednormalisedtouch[58], mergednormalisedtouch[59]);
                oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/touch/discrete");
                lo_send(osc1, oscNamespace.c_str(), "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii", mergeddiscretetouch[0], mergeddiscretetouch[1],mergeddiscretetouch[2],
                mergeddiscretetouch[3],mergeddiscretetouch[4],mergeddiscretetouch[5], mergeddiscretetouch[6], mergeddiscretetouch[7], mergeddiscretetouch[8],
                mergeddiscretetouch[9], mergeddiscretetouch[10], mergeddiscretetouch[11], mergeddiscretetouch[12], mergeddiscretetouch[13], mergeddiscretetouch[14], mergeddiscretetouch[15], mergeddiscretetouch[16],mergeddiscretetouch[17],
                mergeddiscretetouch[18],mergeddiscretetouch[19],mergeddiscretetouch[20], mergeddiscretetouch[21], mergeddiscretetouch[22], mergeddiscretetouch[23],
                mergeddiscretetouch[24], mergeddiscretetouch[25], mergeddiscretetouch[26], mergeddiscretetouch[27], mergeddiscretetouch[28], mergeddiscretetouch[29], mergeddiscretetouch[30], mergeddiscretetouch[31], 
                mergeddiscretetouch[32], mergeddiscretetouch[33], mergeddiscretetouch[34], mergeddiscretetouch[35], mergeddiscretetouch[36], mergeddiscretetouch[37], mergeddiscretetouch[38], mergeddiscretetouch[39], 
                mergeddiscretetouch[40], mergeddiscretetouch[41], mergeddiscretetouch[42], mergeddiscretetouch[43], mergeddiscretetouch[44],mergeddiscretetouch[45],mergeddiscretetouch[46], mergeddiscretetouch[47], 
                mergeddiscretetouch[48], mergeddiscretetouch[49], mergeddiscretetouch[50],mergeddiscretetouch[51], mergeddiscretetouch[52],mergeddiscretetouch[53],
                mergeddiscretetouch[54], mergeddiscretetouch[55], mergeddiscretetouch[56], mergeddiscretetouch[57], mergeddiscretetouch[58], mergeddiscretetouch[59]);
            } else if (TSTICK_SIZE == 90) {
                // Send data from the first board
                lo_send(osc1, oscNamespace.c_str(), "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii", mergedtouch[0], mergedtouch[1],mergedtouch[2],
                mergedtouch[3],mergedtouch[4],mergedtouch[5], mergedtouch[6], mergedtouch[7], mergedtouch[8],
                mergedtouch[9], mergedtouch[10], mergedtouch[11], mergedtouch[12], mergedtouch[13], mergedtouch[14], mergedtouch[15], mergedtouch[16],mergedtouch[17],
                mergedtouch[18],mergedtouch[19],mergedtouch[20], mergedtouch[21], mergedtouch[22], mergedtouch[23],
                mergedtouch[24], mergedtouch[25], mergedtouch[26], mergedtouch[27], mergedtouch[28], mergedtouch[29], mergedtouch[30], mergedtouch[31], 
                mergedtouch[32], mergedtouch[33], mergedtouch[34], mergedtouch[35], mergedtouch[36], mergedtouch[37], mergedtouch[38], mergedtouch[39], 
                mergedtouch[40], mergedtouch[41], mergedtouch[42], mergedtouch[43], mergedtouch[44],mergedtouch[45],mergedtouch[46], mergedtouch[47], 
                mergedtouch[48], mergedtouch[49], mergedtouch[50],mergedtouch[51], mergedtouch[52],mergedtouch[53],
                mergedtouch[54], mergedtouch[55], mergedtouch[56], mergedtouch[57], mergedtouch[58], mergedtouch[59], mergedtouch[60], mergedtouch[61],mergedtouch[62],
                mergedtouch[63],mergedtouch[64],mergedtouch[65], mergedtouch[66], mergedtouch[67], mergedtouch[68],
                mergedtouch[69], mergedtouch[70], mergedtouch[71], mergedtouch[72], mergedtouch[73], mergedtouch[74], mergedtouch[75], mergedtouch[76],mergedtouch[77],
                mergedtouch[78],mergedtouch[79],mergedtouch[80], mergedtouch[81], mergedtouch[82], mergedtouch[83],
                mergedtouch[84], mergedtouch[85], mergedtouch[86], mergedtouch[87], mergedtouch[88], mergedtouch[89]);
                oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/touch/normalised");
                lo_send(osc1, oscNamespace.c_str(), "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii", mergednormalisedtouch[0], mergednormalisedtouch[1],mergednormalisedtouch[2],
                mergednormalisedtouch[3],mergednormalisedtouch[4],mergednormalisedtouch[5], mergednormalisedtouch[6], mergednormalisedtouch[7], mergednormalisedtouch[8],
                mergednormalisedtouch[9], mergednormalisedtouch[10], mergednormalisedtouch[11], mergednormalisedtouch[12], mergednormalisedtouch[13], mergednormalisedtouch[14], mergednormalisedtouch[15], mergednormalisedtouch[16],mergednormalisedtouch[17],
                mergednormalisedtouch[18],mergednormalisedtouch[19],mergednormalisedtouch[20], mergednormalisedtouch[21], mergednormalisedtouch[22], mergednormalisedtouch[23],
                mergednormalisedtouch[24], mergednormalisedtouch[25], mergednormalisedtouch[26], mergednormalisedtouch[27], mergednormalisedtouch[28], mergednormalisedtouch[29], mergednormalisedtouch[30], mergednormalisedtouch[31], 
                mergednormalisedtouch[32], mergednormalisedtouch[33], mergednormalisedtouch[34], mergednormalisedtouch[35], mergednormalisedtouch[36], mergednormalisedtouch[37], mergednormalisedtouch[38], mergednormalisedtouch[39], 
                mergednormalisedtouch[40], mergednormalisedtouch[41], mergednormalisedtouch[42], mergednormalisedtouch[43], mergednormalisedtouch[44],mergednormalisedtouch[45],mergednormalisedtouch[46], mergednormalisedtouch[47], 
                mergednormalisedtouch[48], mergednormalisedtouch[49], mergednormalisedtouch[50],mergednormalisedtouch[51], mergednormalisedtouch[52],mergednormalisedtouch[53],
                mergednormalisedtouch[54], mergednormalisedtouch[55], mergednormalisedtouch[56], mergednormalisedtouch[57], mergednormalisedtouch[58], mergednormalisedtouch[59], mergednormalisedtouch[60], mergednormalisedtouch[61],mergednormalisedtouch[62],
                mergednormalisedtouch[63],mergednormalisedtouch[64],mergednormalisedtouch[65], mergednormalisedtouch[66], mergednormalisedtouch[67], mergednormalisedtouch[68],
                mergednormalisedtouch[69], mergednormalisedtouch[70], mergednormalisedtouch[71], mergednormalisedtouch[72], mergednormalisedtouch[73], mergednormalisedtouch[74], mergednormalisedtouch[75], mergednormalisedtouch[76],mergednormalisedtouch[77],
                mergednormalisedtouch[78],mergednormalisedtouch[79],mergednormalisedtouch[80], mergednormalisedtouch[81], mergednormalisedtouch[82], mergednormalisedtouch[83],
                mergednormalisedtouch[84], mergednormalisedtouch[85], mergednormalisedtouch[86], mergednormalisedtouch[87], mergednormalisedtouch[88], mergednormalisedtouch[89]);
                oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/touch/discrete");
                lo_send(osc1, oscNamespace.c_str(), "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii", mergeddiscretetouch[0], mergeddiscretetouch[1],mergeddiscretetouch[2],
                mergeddiscretetouch[3],mergeddiscretetouch[4],mergeddiscretetouch[5], mergeddiscretetouch[6], mergeddiscretetouch[7], mergeddiscretetouch[8],
                mergeddiscretetouch[9], mergeddiscretetouch[10], mergeddiscretetouch[11], mergeddiscretetouch[12], mergeddiscretetouch[13], mergeddiscretetouch[14], mergeddiscretetouch[15], mergeddiscretetouch[16],mergeddiscretetouch[17],
                mergeddiscretetouch[18],mergeddiscretetouch[19],mergeddiscretetouch[20], mergeddiscretetouch[21], mergeddiscretetouch[22], mergeddiscretetouch[23],
                mergeddiscretetouch[24], mergeddiscretetouch[25], mergeddiscretetouch[26], mergeddiscretetouch[27], mergeddiscretetouch[28], mergeddiscretetouch[29], mergeddiscretetouch[30], mergeddiscretetouch[31], 
                mergeddiscretetouch[32], mergeddiscretetouch[33], mergeddiscretetouch[34], mergeddiscretetouch[35], mergeddiscretetouch[36], mergeddiscretetouch[37], mergeddiscretetouch[38], mergeddiscretetouch[39], 
                mergeddiscretetouch[40], mergeddiscretetouch[41], mergeddiscretetouch[42], mergeddiscretetouch[43], mergeddiscretetouch[44],mergeddiscretetouch[45],mergeddiscretetouch[46], mergeddiscretetouch[47], 
                mergeddiscretetouch[48], mergeddiscretetouch[49], mergeddiscretetouch[50],mergeddiscretetouch[51], mergeddiscretetouch[52],mergeddiscretetouch[53],
                mergeddiscretetouch[54], mergeddiscretetouch[55], mergeddiscretetouch[56], mergeddiscretetouch[57], mergeddiscretetouch[58], mergeddiscretetouch[59], mergeddiscretetouch[60], mergeddiscretetouch[61],mergeddiscretetouch[62],
                mergeddiscretetouch[63],mergeddiscretetouch[64],mergeddiscretetouch[65], mergeddiscretetouch[66], mergeddiscretetouch[67], mergeddiscretetouch[68],
                mergeddiscretetouch[69], mergeddiscretetouch[70], mergeddiscretetouch[71], mergeddiscretetouch[72], mergeddiscretetouch[73], mergeddiscretetouch[74], mergeddiscretetouch[75], mergeddiscretetouch[76],mergeddiscretetouch[77],
                mergeddiscretetouch[78],mergeddiscretetouch[79],mergeddiscretetouch[80], mergeddiscretetouch[81], mergeddiscretetouch[82], mergeddiscretetouch[83],
                mergeddiscretetouch[84], mergeddiscretetouch[85], mergeddiscretetouch[86], mergeddiscretetouch[87], mergeddiscretetouch[88], mergeddiscretetouch[89]);
            } else if (TSTICK_SIZE == 120) {
                // Send data from the first board
                lo_send(osc1, oscNamespace.c_str(), "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii", mergedtouch[0], mergedtouch[1],mergedtouch[2],
                mergedtouch[3],mergedtouch[4],mergedtouch[5], mergedtouch[6], mergedtouch[7], mergedtouch[8],
                mergedtouch[9], mergedtouch[10], mergedtouch[11], mergedtouch[12], mergedtouch[13], mergedtouch[14], mergedtouch[15], mergedtouch[16],mergedtouch[17],
                mergedtouch[18],mergedtouch[19],mergedtouch[20], mergedtouch[21], mergedtouch[22], mergedtouch[23],
                mergedtouch[24], mergedtouch[25], mergedtouch[26], mergedtouch[27], mergedtouch[28], mergedtouch[29], mergedtouch[30], mergedtouch[31], 
                mergedtouch[32], mergedtouch[33], mergedtouch[34], mergedtouch[35], mergedtouch[36], mergedtouch[37], mergedtouch[38], mergedtouch[39], 
                mergedtouch[40], mergedtouch[41], mergedtouch[42], mergedtouch[43], mergedtouch[44],mergedtouch[45],mergedtouch[46], mergedtouch[47], 
                mergedtouch[48], mergedtouch[49], mergedtouch[50],mergedtouch[51], mergedtouch[52],mergedtouch[53],
                mergedtouch[54], mergedtouch[55], mergedtouch[56], mergedtouch[57], mergedtouch[58], mergedtouch[59], mergedtouch[60], mergedtouch[61],mergedtouch[62],
                mergedtouch[63],mergedtouch[64],mergedtouch[65], mergedtouch[66], mergedtouch[67], mergedtouch[68],
                mergedtouch[69], mergedtouch[70], mergedtouch[71], mergedtouch[72], mergedtouch[73], mergedtouch[74], mergedtouch[75], mergedtouch[76],mergedtouch[77],
                mergedtouch[78],mergedtouch[79],mergedtouch[80], mergedtouch[81], mergedtouch[82], mergedtouch[83],
                mergedtouch[84], mergedtouch[85], mergedtouch[86], mergedtouch[87], mergedtouch[88], mergedtouch[89], mergedtouch[90], mergedtouch[91],mergedtouch[92],
                mergedtouch[93],mergedtouch[94],mergedtouch[95], mergedtouch[96], mergedtouch[97], mergedtouch[98],
                mergedtouch[99], mergedtouch[100], mergedtouch[101], mergedtouch[102], mergedtouch[103], mergedtouch[104], mergedtouch[105], mergedtouch[106],mergedtouch[107],
                mergedtouch[108],mergedtouch[109],mergedtouch[110], mergedtouch[111], mergedtouch[112], mergedtouch[113],
                mergedtouch[114], mergedtouch[115], mergedtouch[116], mergedtouch[117], mergedtouch[118], mergedtouch[119]);
                oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/touch/normalised");
                lo_send(osc1, oscNamespace.c_str(), "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii", mergednormalisedtouch[0], mergednormalisedtouch[1],mergednormalisedtouch[2],
                mergednormalisedtouch[3],mergednormalisedtouch[4],mergednormalisedtouch[5], mergednormalisedtouch[6], mergednormalisedtouch[7], mergednormalisedtouch[8],
                mergednormalisedtouch[9], mergednormalisedtouch[10], mergednormalisedtouch[11], mergednormalisedtouch[12], mergednormalisedtouch[13], mergednormalisedtouch[14], mergednormalisedtouch[15], mergednormalisedtouch[16],mergednormalisedtouch[17],
                mergednormalisedtouch[18],mergednormalisedtouch[19],mergednormalisedtouch[20], mergednormalisedtouch[21], mergednormalisedtouch[22], mergednormalisedtouch[23],
                mergednormalisedtouch[24], mergednormalisedtouch[25], mergednormalisedtouch[26], mergednormalisedtouch[27], mergednormalisedtouch[28], mergednormalisedtouch[29], mergednormalisedtouch[30], mergednormalisedtouch[31], 
                mergednormalisedtouch[32], mergednormalisedtouch[33], mergednormalisedtouch[34], mergednormalisedtouch[35], mergednormalisedtouch[36], mergednormalisedtouch[37], mergednormalisedtouch[38], mergednormalisedtouch[39], 
                mergednormalisedtouch[40], mergednormalisedtouch[41], mergednormalisedtouch[42], mergednormalisedtouch[43], mergednormalisedtouch[44],mergednormalisedtouch[45],mergednormalisedtouch[46], mergednormalisedtouch[47], 
                mergednormalisedtouch[48], mergednormalisedtouch[49], mergednormalisedtouch[50],mergednormalisedtouch[51], mergednormalisedtouch[52],mergednormalisedtouch[53],
                mergednormalisedtouch[54], mergednormalisedtouch[55], mergednormalisedtouch[56], mergednormalisedtouch[57], mergednormalisedtouch[58], mergednormalisedtouch[59], mergednormalisedtouch[60], mergednormalisedtouch[61],mergednormalisedtouch[62],
                mergednormalisedtouch[63],mergednormalisedtouch[64],mergednormalisedtouch[65], mergednormalisedtouch[66], mergednormalisedtouch[67], mergednormalisedtouch[68],
                mergednormalisedtouch[69], mergednormalisedtouch[70], mergednormalisedtouch[71], mergednormalisedtouch[72], mergednormalisedtouch[73], mergednormalisedtouch[74], mergednormalisedtouch[75], mergednormalisedtouch[76],mergednormalisedtouch[77],
                mergednormalisedtouch[78],mergednormalisedtouch[79],mergednormalisedtouch[80], mergednormalisedtouch[81], mergednormalisedtouch[82], mergednormalisedtouch[83],
                mergednormalisedtouch[84], mergednormalisedtouch[85], mergednormalisedtouch[86], mergednormalisedtouch[87], mergednormalisedtouch[88], mergednormalisedtouch[89], mergednormalisedtouch[90], mergednormalisedtouch[91],mergednormalisedtouch[92],
                mergednormalisedtouch[93],mergednormalisedtouch[94],mergednormalisedtouch[95], mergednormalisedtouch[96], mergednormalisedtouch[97], mergednormalisedtouch[98],
                mergednormalisedtouch[99], mergednormalisedtouch[100], mergednormalisedtouch[101], mergednormalisedtouch[102], mergednormalisedtouch[103], mergednormalisedtouch[104], mergednormalisedtouch[105], mergednormalisedtouch[106],mergednormalisedtouch[107],
                mergednormalisedtouch[108],mergednormalisedtouch[109],mergednormalisedtouch[110], mergednormalisedtouch[111], mergednormalisedtouch[112], mergednormalisedtouch[113],
                mergednormalisedtouch[114], mergednormalisedtouch[115], mergednormalisedtouch[116], mergednormalisedtouch[117], mergednormalisedtouch[118], mergednormalisedtouch[119]);
                oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/touch/discrete");
                lo_send(osc1, oscNamespace.c_str(), "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii", mergeddiscretetouch[0], mergeddiscretetouch[1],mergeddiscretetouch[2],
                mergeddiscretetouch[3],mergeddiscretetouch[4],mergeddiscretetouch[5], mergeddiscretetouch[6], mergeddiscretetouch[7], mergeddiscretetouch[8],
                mergeddiscretetouch[9], mergeddiscretetouch[10], mergeddiscretetouch[11], mergeddiscretetouch[12], mergeddiscretetouch[13], mergeddiscretetouch[14], mergeddiscretetouch[15], mergeddiscretetouch[16],mergeddiscretetouch[17],
                mergeddiscretetouch[18],mergeddiscretetouch[19],mergeddiscretetouch[20], mergeddiscretetouch[21], mergeddiscretetouch[22], mergeddiscretetouch[23],
                mergeddiscretetouch[24], mergeddiscretetouch[25], mergeddiscretetouch[26], mergeddiscretetouch[27], mergeddiscretetouch[28], mergeddiscretetouch[29], mergeddiscretetouch[30], mergeddiscretetouch[31], 
                mergeddiscretetouch[32], mergeddiscretetouch[33], mergeddiscretetouch[34], mergeddiscretetouch[35], mergeddiscretetouch[36], mergeddiscretetouch[37], mergeddiscretetouch[38], mergeddiscretetouch[39], 
                mergeddiscretetouch[40], mergeddiscretetouch[41], mergeddiscretetouch[42], mergeddiscretetouch[43], mergeddiscretetouch[44],mergeddiscretetouch[45],mergeddiscretetouch[46], mergeddiscretetouch[47], 
                mergeddiscretetouch[48], mergeddiscretetouch[49], mergeddiscretetouch[50],mergeddiscretetouch[51], mergeddiscretetouch[52],mergeddiscretetouch[53],
                mergeddiscretetouch[54], mergeddiscretetouch[55], mergeddiscretetouch[56], mergeddiscretetouch[57], mergeddiscretetouch[58], mergeddiscretetouch[59], mergeddiscretetouch[60], mergeddiscretetouch[61],mergeddiscretetouch[62],
                mergeddiscretetouch[63],mergeddiscretetouch[64],mergeddiscretetouch[65], mergeddiscretetouch[66], mergeddiscretetouch[67], mergeddiscretetouch[68],
                mergeddiscretetouch[69], mergeddiscretetouch[70], mergeddiscretetouch[71], mergeddiscretetouch[72], mergeddiscretetouch[73], mergeddiscretetouch[74], mergeddiscretetouch[75], mergeddiscretetouch[76],mergeddiscretetouch[77],
                mergeddiscretetouch[78],mergeddiscretetouch[79],mergeddiscretetouch[80], mergeddiscretetouch[81], mergeddiscretetouch[82], mergeddiscretetouch[83],
                mergeddiscretetouch[84], mergeddiscretetouch[85], mergeddiscretetouch[86], mergeddiscretetouch[87], mergeddiscretetouch[88], mergeddiscretetouch[89], mergeddiscretetouch[90], mergeddiscretetouch[91],mergeddiscretetouch[92],
                mergeddiscretetouch[93],mergeddiscretetouch[94],mergeddiscretetouch[95], mergeddiscretetouch[96], mergeddiscretetouch[97], mergeddiscretetouch[98],
                mergeddiscretetouch[99], mergeddiscretetouch[100], mergeddiscretetouch[101], mergeddiscretetouch[102], mergeddiscretetouch[103], mergeddiscretetouch[104], mergeddiscretetouch[105], mergeddiscretetouch[106],mergeddiscretetouch[107],
                mergeddiscretetouch[108],mergeddiscretetouch[109],mergeddiscretetouch[110], mergeddiscretetouch[111], mergeddiscretetouch[112], mergeddiscretetouch[113],
                mergeddiscretetouch[114], mergeddiscretetouch[115], mergeddiscretetouch[116], mergeddiscretetouch[117], mergeddiscretetouch[118], mergeddiscretetouch[119]);
            }

            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/touch/all");
            lo_send(osc1, oscNamespace.c_str(), "f", gestures.touchAll);
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/touch/top");
            lo_send(osc1, oscNamespace.c_str(), "f", gestures.touchTop);
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/touch/middle");
            lo_send(osc1, oscNamespace.c_str(), "f", gestures.touchMiddle);
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/touch/bottom");
            lo_send(osc1, oscNamespace.c_str(), "f", gestures.touchBottom);

            // Reset touch event until next interrupt
            #ifdef touch_ENCHANTI
            // event.touchReady = false;
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/touch/scantime");
            lo_send(osc1, oscNamespace.c_str(), "i", touch.scantime);
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/touch/polltime");
            lo_send(osc1, oscNamespace.c_str(), "i", touch.polltime);
            #endif
        }
        if (event.mimu) {
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "raw/accl");
            lo_send(osc1, oscNamespace.c_str(), "fff", sensors.accl[0], sensors.accl[1], sensors.accl[2]);
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "raw/gyro");
            lo_send(osc1, oscNamespace.c_str(), "fff", sensors.gyro[0], sensors.gyro[1], sensors.gyro[2]);
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "raw/magn");
            lo_send(osc1, oscNamespace.c_str(), "fff", sensors.magn[0], sensors.magn[1], sensors.magn[2]);
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "orientation");
            lo_send(osc1, oscNamespace.c_str(), "ffff", sensors.quat[0], sensors.quat[1], sensors.quat[2], sensors.quat[3]);
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "ypr");
            lo_send(osc1, oscNamespace.c_str(), "fff", sensors.ypr[0], sensors.ypr[1], sensors.ypr[2]);        
            // Reset mimu event
            event.mimu = false;        
        }
        if (event.brush) {
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/brush");
            lo_send(osc1, oscNamespace.c_str(), "f", sensors.brush);
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/multibrush");
            lo_send(osc1, oscNamespace.c_str(), "fff", sensors.multibrush[0], sensors.multibrush[1], sensors.multibrush[2]);
        }
        if (event.rub) {
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/rub");
            lo_send(osc1, oscNamespace.c_str(), "f", sensors.rub);
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/multirub");
            lo_send(osc1, oscNamespace.c_str(), "fff", sensors.multirub[0], sensors.multirub[1], sensors.multirub[2]);
        }
        if (event.shake) {
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/shakexyz");
            lo_send(osc1, oscNamespace.c_str(), "fff", sensors.shake[0], sensors.shake[1], sensors.shake[2]);
        }
        if (event.jab) {
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/jabxyz");
            lo_send(osc1, oscNamespace.c_str(), "fff", sensors.jab[0], sensors.jab[1], sensors.jab[2]);
        }
        if (event.count) {
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/button/count");
            lo_send(osc1, oscNamespace.c_str(), "i", sensors.count);
        }
        if (event.tap) {
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/button/tap");
            lo_send(osc1, oscNamespace.c_str(), "i", sensors.tap);
        }
        if (event.dtap) {
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/button/dtap");
            lo_send(osc1, oscNamespace.c_str(), "i", sensors.dtap);
        }
        if (event.ttap) {
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/button/ttap");
            lo_send(osc1, oscNamespace.c_str(), "i", sensors.ttap);
        }
        if (event.battery) {
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "battery/percentage");
            lo_send(osc1, oscNamespace.c_str(), "i", battery.percentage);
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "battery/capacity");
            lo_send(osc1, oscNamespace.c_str(), "i", battery.capacity);
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "battery/status");
            lo_send(osc1, oscNamespace.c_str(), "i", battery.status);       
        }
        if (event.current) {
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "battery/current");
            lo_send(osc1, oscNamespace.c_str(), "i", battery.current);
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "battery/tte");
            lo_send(osc1, oscNamespace.c_str(), "f", battery.TTE);
        }
        if (event.voltage) {
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "battery/voltage");
            lo_send(osc1, oscNamespace.c_str(), "f", battery.voltage);
        }
        }    
}
void updateOSC2() {
    if (puara.IP2_ready()) {
        // Continuously send FSR data
        oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "raw/fsr");
        lo_send(osc2, oscNamespace.c_str(), "i", sensors.fsr);
        oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/squeeze");
        lo_send(osc2, oscNamespace.c_str(), "f", sensors.squeeze);

        // Send discrete messages
        if (event.touchReady) {
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "raw/capsense");
            if (TSTICK_SIZE == 30) {
                lo_send(osc2, oscNamespace.c_str(), "iiiiiiiiiiiiiiiiiiiiiiiiiiiiii", mergedtouch[0], mergedtouch[1],mergedtouch[2],
                mergedtouch[3],mergedtouch[4],mergedtouch[5], mergedtouch[6], mergedtouch[7], mergedtouch[8],
                mergedtouch[9], mergedtouch[10], mergedtouch[11], mergedtouch[12], mergedtouch[13], mergedtouch[14], mergedtouch[15], mergedtouch[16],mergedtouch[17],
                mergedtouch[18],mergedtouch[19],mergedtouch[20], mergedtouch[21], mergedtouch[22], mergedtouch[23],
                mergedtouch[24], mergedtouch[25], mergedtouch[26], mergedtouch[27], mergedtouch[28], mergedtouch[29]);
                oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/touch/normalised");
                lo_send(osc2, oscNamespace.c_str(), "iiiiiiiiiiiiiiiiiiiiiiiiiiiiii", mergednormalisedtouch[0], mergednormalisedtouch[1],mergednormalisedtouch[2],
                mergednormalisedtouch[3],mergednormalisedtouch[4],mergednormalisedtouch[5], mergednormalisedtouch[6], mergednormalisedtouch[7], mergednormalisedtouch[8],
                mergednormalisedtouch[9], mergednormalisedtouch[10], mergednormalisedtouch[11], mergednormalisedtouch[12], mergednormalisedtouch[13], mergednormalisedtouch[14], mergednormalisedtouch[15], mergednormalisedtouch[16],mergednormalisedtouch[17],
                mergednormalisedtouch[18],mergednormalisedtouch[19],mergednormalisedtouch[20], mergednormalisedtouch[21], mergednormalisedtouch[22], mergednormalisedtouch[23],
                mergednormalisedtouch[24], mergednormalisedtouch[25], mergednormalisedtouch[26], mergednormalisedtouch[27], mergednormalisedtouch[28], mergednormalisedtouch[29]);
                oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/touch/discrete");
                lo_send(osc2, oscNamespace.c_str(), "iiiiiiiiiiiiiiiiiiiiiiiiiiiiii", mergeddiscretetouch[0], mergeddiscretetouch[1],mergeddiscretetouch[2],
                mergeddiscretetouch[3],mergeddiscretetouch[4],mergeddiscretetouch[5], mergeddiscretetouch[6], mergeddiscretetouch[7], mergeddiscretetouch[8],
                mergeddiscretetouch[9], mergeddiscretetouch[10], mergeddiscretetouch[11], mergeddiscretetouch[12], mergeddiscretetouch[13], mergeddiscretetouch[14], mergeddiscretetouch[15], mergeddiscretetouch[16],mergeddiscretetouch[17],
                mergeddiscretetouch[18],mergeddiscretetouch[19],mergeddiscretetouch[20], mergeddiscretetouch[21], mergeddiscretetouch[22], mergeddiscretetouch[23],
                mergeddiscretetouch[24], mergeddiscretetouch[25], mergeddiscretetouch[26], mergeddiscretetouch[27], mergeddiscretetouch[28], mergeddiscretetouch[29]);
            } else if (TSTICK_SIZE == 60) {
                // Send data from the first board
                lo_send(osc2, oscNamespace.c_str(), "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii", mergedtouch[0], mergedtouch[1],mergedtouch[2],
                mergedtouch[3],mergedtouch[4],mergedtouch[5], mergedtouch[6], mergedtouch[7], mergedtouch[8],
                mergedtouch[9], mergedtouch[10], mergedtouch[11], mergedtouch[12], mergedtouch[13], mergedtouch[14], mergedtouch[15], mergedtouch[16],mergedtouch[17],
                mergedtouch[18],mergedtouch[19],mergedtouch[20], mergedtouch[21], mergedtouch[22], mergedtouch[23],
                mergedtouch[24], mergedtouch[25], mergedtouch[26], mergedtouch[27], mergedtouch[28], mergedtouch[29], mergedtouch[30], mergedtouch[31], 
                mergedtouch[32], mergedtouch[33], mergedtouch[34], mergedtouch[35], mergedtouch[36], mergedtouch[37], mergedtouch[38], mergedtouch[39], 
                mergedtouch[40], mergedtouch[41], mergedtouch[42], mergedtouch[43], mergedtouch[44],mergedtouch[45],mergedtouch[46], mergedtouch[47], 
                mergedtouch[48], mergedtouch[49], mergedtouch[50],mergedtouch[51], mergedtouch[52],mergedtouch[53],
                mergedtouch[54], mergedtouch[55], mergedtouch[56], mergedtouch[57], mergedtouch[58], mergedtouch[59]);
                oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/touch/normalised");
                lo_send(osc2, oscNamespace.c_str(), "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii", mergednormalisedtouch[0], mergednormalisedtouch[1],mergednormalisedtouch[2],
                mergednormalisedtouch[3],mergednormalisedtouch[4],mergednormalisedtouch[5], mergednormalisedtouch[6], mergednormalisedtouch[7], mergednormalisedtouch[8],
                mergednormalisedtouch[9], mergednormalisedtouch[10], mergednormalisedtouch[11], mergednormalisedtouch[12], mergednormalisedtouch[13], mergednormalisedtouch[14], mergednormalisedtouch[15], mergednormalisedtouch[16],mergednormalisedtouch[17],
                mergednormalisedtouch[18],mergednormalisedtouch[19],mergednormalisedtouch[20], mergednormalisedtouch[21], mergednormalisedtouch[22], mergednormalisedtouch[23],
                mergednormalisedtouch[24], mergednormalisedtouch[25], mergednormalisedtouch[26], mergednormalisedtouch[27], mergednormalisedtouch[28], mergednormalisedtouch[29], mergednormalisedtouch[30], mergednormalisedtouch[31], 
                mergednormalisedtouch[32], mergednormalisedtouch[33], mergednormalisedtouch[34], mergednormalisedtouch[35], mergednormalisedtouch[36], mergednormalisedtouch[37], mergednormalisedtouch[38], mergednormalisedtouch[39], 
                mergednormalisedtouch[40], mergednormalisedtouch[41], mergednormalisedtouch[42], mergednormalisedtouch[43], mergednormalisedtouch[44],mergednormalisedtouch[45],mergednormalisedtouch[46], mergednormalisedtouch[47], 
                mergednormalisedtouch[48], mergednormalisedtouch[49], mergednormalisedtouch[50],mergednormalisedtouch[51], mergednormalisedtouch[52],mergednormalisedtouch[53],
                mergednormalisedtouch[54], mergednormalisedtouch[55], mergednormalisedtouch[56], mergednormalisedtouch[57], mergednormalisedtouch[58], mergednormalisedtouch[59]);
                oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/touch/discrete");
                lo_send(osc2, oscNamespace.c_str(), "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii", mergeddiscretetouch[0], mergeddiscretetouch[1],mergeddiscretetouch[2],
                mergeddiscretetouch[3],mergeddiscretetouch[4],mergeddiscretetouch[5], mergeddiscretetouch[6], mergeddiscretetouch[7], mergeddiscretetouch[8],
                mergeddiscretetouch[9], mergeddiscretetouch[10], mergeddiscretetouch[11], mergeddiscretetouch[12], mergeddiscretetouch[13], mergeddiscretetouch[14], mergeddiscretetouch[15], mergeddiscretetouch[16],mergeddiscretetouch[17],
                mergeddiscretetouch[18],mergeddiscretetouch[19],mergeddiscretetouch[20], mergeddiscretetouch[21], mergeddiscretetouch[22], mergeddiscretetouch[23],
                mergeddiscretetouch[24], mergeddiscretetouch[25], mergeddiscretetouch[26], mergeddiscretetouch[27], mergeddiscretetouch[28], mergeddiscretetouch[29], mergeddiscretetouch[30], mergeddiscretetouch[31], 
                mergeddiscretetouch[32], mergeddiscretetouch[33], mergeddiscretetouch[34], mergeddiscretetouch[35], mergeddiscretetouch[36], mergeddiscretetouch[37], mergeddiscretetouch[38], mergeddiscretetouch[39], 
                mergeddiscretetouch[40], mergeddiscretetouch[41], mergeddiscretetouch[42], mergeddiscretetouch[43], mergeddiscretetouch[44],mergeddiscretetouch[45],mergeddiscretetouch[46], mergeddiscretetouch[47], 
                mergeddiscretetouch[48], mergeddiscretetouch[49], mergeddiscretetouch[50],mergeddiscretetouch[51], mergeddiscretetouch[52],mergeddiscretetouch[53],
                mergeddiscretetouch[54], mergeddiscretetouch[55], mergeddiscretetouch[56], mergeddiscretetouch[57], mergeddiscretetouch[58], mergeddiscretetouch[59]);
            } else if (TSTICK_SIZE == 90) {
                // Send data from the first board
                lo_send(osc2, oscNamespace.c_str(), "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii", mergedtouch[0], mergedtouch[1],mergedtouch[2],
                mergedtouch[3],mergedtouch[4],mergedtouch[5], mergedtouch[6], mergedtouch[7], mergedtouch[8],
                mergedtouch[9], mergedtouch[10], mergedtouch[11], mergedtouch[12], mergedtouch[13], mergedtouch[14], mergedtouch[15], mergedtouch[16],mergedtouch[17],
                mergedtouch[18],mergedtouch[19],mergedtouch[20], mergedtouch[21], mergedtouch[22], mergedtouch[23],
                mergedtouch[24], mergedtouch[25], mergedtouch[26], mergedtouch[27], mergedtouch[28], mergedtouch[29], mergedtouch[30], mergedtouch[31], 
                mergedtouch[32], mergedtouch[33], mergedtouch[34], mergedtouch[35], mergedtouch[36], mergedtouch[37], mergedtouch[38], mergedtouch[39], 
                mergedtouch[40], mergedtouch[41], mergedtouch[42], mergedtouch[43], mergedtouch[44],mergedtouch[45],mergedtouch[46], mergedtouch[47], 
                mergedtouch[48], mergedtouch[49], mergedtouch[50],mergedtouch[51], mergedtouch[52],mergedtouch[53],
                mergedtouch[54], mergedtouch[55], mergedtouch[56], mergedtouch[57], mergedtouch[58], mergedtouch[59], mergedtouch[60], mergedtouch[61],mergedtouch[62],
                mergedtouch[63],mergedtouch[64],mergedtouch[65], mergedtouch[66], mergedtouch[67], mergedtouch[68],
                mergedtouch[69], mergedtouch[70], mergedtouch[71], mergedtouch[72], mergedtouch[73], mergedtouch[74], mergedtouch[75], mergedtouch[76],mergedtouch[77],
                mergedtouch[78],mergedtouch[79],mergedtouch[80], mergedtouch[81], mergedtouch[82], mergedtouch[83],
                mergedtouch[84], mergedtouch[85], mergedtouch[86], mergedtouch[87], mergedtouch[88], mergedtouch[89]);
                oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/touch/normalised");
                lo_send(osc2, oscNamespace.c_str(), "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii", mergednormalisedtouch[0], mergednormalisedtouch[1],mergednormalisedtouch[2],
                mergednormalisedtouch[3],mergednormalisedtouch[4],mergednormalisedtouch[5], mergednormalisedtouch[6], mergednormalisedtouch[7], mergednormalisedtouch[8],
                mergednormalisedtouch[9], mergednormalisedtouch[10], mergednormalisedtouch[11], mergednormalisedtouch[12], mergednormalisedtouch[13], mergednormalisedtouch[14], mergednormalisedtouch[15], mergednormalisedtouch[16],mergednormalisedtouch[17],
                mergednormalisedtouch[18],mergednormalisedtouch[19],mergednormalisedtouch[20], mergednormalisedtouch[21], mergednormalisedtouch[22], mergednormalisedtouch[23],
                mergednormalisedtouch[24], mergednormalisedtouch[25], mergednormalisedtouch[26], mergednormalisedtouch[27], mergednormalisedtouch[28], mergednormalisedtouch[29], mergednormalisedtouch[30], mergednormalisedtouch[31], 
                mergednormalisedtouch[32], mergednormalisedtouch[33], mergednormalisedtouch[34], mergednormalisedtouch[35], mergednormalisedtouch[36], mergednormalisedtouch[37], mergednormalisedtouch[38], mergednormalisedtouch[39], 
                mergednormalisedtouch[40], mergednormalisedtouch[41], mergednormalisedtouch[42], mergednormalisedtouch[43], mergednormalisedtouch[44],mergednormalisedtouch[45],mergednormalisedtouch[46], mergednormalisedtouch[47], 
                mergednormalisedtouch[48], mergednormalisedtouch[49], mergednormalisedtouch[50],mergednormalisedtouch[51], mergednormalisedtouch[52],mergednormalisedtouch[53],
                mergednormalisedtouch[54], mergednormalisedtouch[55], mergednormalisedtouch[56], mergednormalisedtouch[57], mergednormalisedtouch[58], mergednormalisedtouch[59], mergednormalisedtouch[60], mergednormalisedtouch[61],mergednormalisedtouch[62],
                mergednormalisedtouch[63],mergednormalisedtouch[64],mergednormalisedtouch[65], mergednormalisedtouch[66], mergednormalisedtouch[67], mergednormalisedtouch[68],
                mergednormalisedtouch[69], mergednormalisedtouch[70], mergednormalisedtouch[71], mergednormalisedtouch[72], mergednormalisedtouch[73], mergednormalisedtouch[74], mergednormalisedtouch[75], mergednormalisedtouch[76],mergednormalisedtouch[77],
                mergednormalisedtouch[78],mergednormalisedtouch[79],mergednormalisedtouch[80], mergednormalisedtouch[81], mergednormalisedtouch[82], mergednormalisedtouch[83],
                mergednormalisedtouch[84], mergednormalisedtouch[85], mergednormalisedtouch[86], mergednormalisedtouch[87], mergednormalisedtouch[88], mergednormalisedtouch[89]);
                oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/touch/discrete");
                lo_send(osc2, oscNamespace.c_str(), "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii", mergeddiscretetouch[0], mergeddiscretetouch[1],mergeddiscretetouch[2],
                mergeddiscretetouch[3],mergeddiscretetouch[4],mergeddiscretetouch[5], mergeddiscretetouch[6], mergeddiscretetouch[7], mergeddiscretetouch[8],
                mergeddiscretetouch[9], mergeddiscretetouch[10], mergeddiscretetouch[11], mergeddiscretetouch[12], mergeddiscretetouch[13], mergeddiscretetouch[14], mergeddiscretetouch[15], mergeddiscretetouch[16],mergeddiscretetouch[17],
                mergeddiscretetouch[18],mergeddiscretetouch[19],mergeddiscretetouch[20], mergeddiscretetouch[21], mergeddiscretetouch[22], mergeddiscretetouch[23],
                mergeddiscretetouch[24], mergeddiscretetouch[25], mergeddiscretetouch[26], mergeddiscretetouch[27], mergeddiscretetouch[28], mergeddiscretetouch[29], mergeddiscretetouch[30], mergeddiscretetouch[31], 
                mergeddiscretetouch[32], mergeddiscretetouch[33], mergeddiscretetouch[34], mergeddiscretetouch[35], mergeddiscretetouch[36], mergeddiscretetouch[37], mergeddiscretetouch[38], mergeddiscretetouch[39], 
                mergeddiscretetouch[40], mergeddiscretetouch[41], mergeddiscretetouch[42], mergeddiscretetouch[43], mergeddiscretetouch[44],mergeddiscretetouch[45],mergeddiscretetouch[46], mergeddiscretetouch[47], 
                mergeddiscretetouch[48], mergeddiscretetouch[49], mergeddiscretetouch[50],mergeddiscretetouch[51], mergeddiscretetouch[52],mergeddiscretetouch[53],
                mergeddiscretetouch[54], mergeddiscretetouch[55], mergeddiscretetouch[56], mergeddiscretetouch[57], mergeddiscretetouch[58], mergeddiscretetouch[59], mergeddiscretetouch[60], mergeddiscretetouch[61],mergeddiscretetouch[62],
                mergeddiscretetouch[63],mergeddiscretetouch[64],mergeddiscretetouch[65], mergeddiscretetouch[66], mergeddiscretetouch[67], mergeddiscretetouch[68],
                mergeddiscretetouch[69], mergeddiscretetouch[70], mergeddiscretetouch[71], mergeddiscretetouch[72], mergeddiscretetouch[73], mergeddiscretetouch[74], mergeddiscretetouch[75], mergeddiscretetouch[76],mergeddiscretetouch[77],
                mergeddiscretetouch[78],mergeddiscretetouch[79],mergeddiscretetouch[80], mergeddiscretetouch[81], mergeddiscretetouch[82], mergeddiscretetouch[83],
                mergeddiscretetouch[84], mergeddiscretetouch[85], mergeddiscretetouch[86], mergeddiscretetouch[87], mergeddiscretetouch[88], mergeddiscretetouch[89]);
            } else if (TSTICK_SIZE == 120) {
                // Send data from the first board
                lo_send(osc2, oscNamespace.c_str(), "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii", mergedtouch[0], mergedtouch[1],mergedtouch[2],
                mergedtouch[3],mergedtouch[4],mergedtouch[5], mergedtouch[6], mergedtouch[7], mergedtouch[8],
                mergedtouch[9], mergedtouch[10], mergedtouch[11], mergedtouch[12], mergedtouch[13], mergedtouch[14], mergedtouch[15], mergedtouch[16],mergedtouch[17],
                mergedtouch[18],mergedtouch[19],mergedtouch[20], mergedtouch[21], mergedtouch[22], mergedtouch[23],
                mergedtouch[24], mergedtouch[25], mergedtouch[26], mergedtouch[27], mergedtouch[28], mergedtouch[29], mergedtouch[30], mergedtouch[31], 
                mergedtouch[32], mergedtouch[33], mergedtouch[34], mergedtouch[35], mergedtouch[36], mergedtouch[37], mergedtouch[38], mergedtouch[39], 
                mergedtouch[40], mergedtouch[41], mergedtouch[42], mergedtouch[43], mergedtouch[44],mergedtouch[45],mergedtouch[46], mergedtouch[47], 
                mergedtouch[48], mergedtouch[49], mergedtouch[50],mergedtouch[51], mergedtouch[52],mergedtouch[53],
                mergedtouch[54], mergedtouch[55], mergedtouch[56], mergedtouch[57], mergedtouch[58], mergedtouch[59], mergedtouch[60], mergedtouch[61],mergedtouch[62],
                mergedtouch[63],mergedtouch[64],mergedtouch[65], mergedtouch[66], mergedtouch[67], mergedtouch[68],
                mergedtouch[69], mergedtouch[70], mergedtouch[71], mergedtouch[72], mergedtouch[73], mergedtouch[74], mergedtouch[75], mergedtouch[76],mergedtouch[77],
                mergedtouch[78],mergedtouch[79],mergedtouch[80], mergedtouch[81], mergedtouch[82], mergedtouch[83],
                mergedtouch[84], mergedtouch[85], mergedtouch[86], mergedtouch[87], mergedtouch[88], mergedtouch[89], mergedtouch[90], mergedtouch[91],mergedtouch[92],
                mergedtouch[93],mergedtouch[94],mergedtouch[95], mergedtouch[96], mergedtouch[97], mergedtouch[98],
                mergedtouch[99], mergedtouch[100], mergedtouch[101], mergedtouch[102], mergedtouch[103], mergedtouch[104], mergedtouch[105], mergedtouch[106],mergedtouch[107],
                mergedtouch[108],mergedtouch[109],mergedtouch[110], mergedtouch[111], mergedtouch[112], mergedtouch[113],
                mergedtouch[114], mergedtouch[115], mergedtouch[116], mergedtouch[117], mergedtouch[118], mergedtouch[119]);
                oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/touch/normalised");
                lo_send(osc2, oscNamespace.c_str(), "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii", mergednormalisedtouch[0], mergednormalisedtouch[1],mergednormalisedtouch[2],
                mergednormalisedtouch[3],mergednormalisedtouch[4],mergednormalisedtouch[5], mergednormalisedtouch[6], mergednormalisedtouch[7], mergednormalisedtouch[8],
                mergednormalisedtouch[9], mergednormalisedtouch[10], mergednormalisedtouch[11], mergednormalisedtouch[12], mergednormalisedtouch[13], mergednormalisedtouch[14], mergednormalisedtouch[15], mergednormalisedtouch[16],mergednormalisedtouch[17],
                mergednormalisedtouch[18],mergednormalisedtouch[19],mergednormalisedtouch[20], mergednormalisedtouch[21], mergednormalisedtouch[22], mergednormalisedtouch[23],
                mergednormalisedtouch[24], mergednormalisedtouch[25], mergednormalisedtouch[26], mergednormalisedtouch[27], mergednormalisedtouch[28], mergednormalisedtouch[29], mergednormalisedtouch[30], mergednormalisedtouch[31], 
                mergednormalisedtouch[32], mergednormalisedtouch[33], mergednormalisedtouch[34], mergednormalisedtouch[35], mergednormalisedtouch[36], mergednormalisedtouch[37], mergednormalisedtouch[38], mergednormalisedtouch[39], 
                mergednormalisedtouch[40], mergednormalisedtouch[41], mergednormalisedtouch[42], mergednormalisedtouch[43], mergednormalisedtouch[44],mergednormalisedtouch[45],mergednormalisedtouch[46], mergednormalisedtouch[47], 
                mergednormalisedtouch[48], mergednormalisedtouch[49], mergednormalisedtouch[50],mergednormalisedtouch[51], mergednormalisedtouch[52],mergednormalisedtouch[53],
                mergednormalisedtouch[54], mergednormalisedtouch[55], mergednormalisedtouch[56], mergednormalisedtouch[57], mergednormalisedtouch[58], mergednormalisedtouch[59], mergednormalisedtouch[60], mergednormalisedtouch[61],mergednormalisedtouch[62],
                mergednormalisedtouch[63],mergednormalisedtouch[64],mergednormalisedtouch[65], mergednormalisedtouch[66], mergednormalisedtouch[67], mergednormalisedtouch[68],
                mergednormalisedtouch[69], mergednormalisedtouch[70], mergednormalisedtouch[71], mergednormalisedtouch[72], mergednormalisedtouch[73], mergednormalisedtouch[74], mergednormalisedtouch[75], mergednormalisedtouch[76],mergednormalisedtouch[77],
                mergednormalisedtouch[78],mergednormalisedtouch[79],mergednormalisedtouch[80], mergednormalisedtouch[81], mergednormalisedtouch[82], mergednormalisedtouch[83],
                mergednormalisedtouch[84], mergednormalisedtouch[85], mergednormalisedtouch[86], mergednormalisedtouch[87], mergednormalisedtouch[88], mergednormalisedtouch[89], mergednormalisedtouch[90], mergednormalisedtouch[91],mergednormalisedtouch[92],
                mergednormalisedtouch[93],mergednormalisedtouch[94],mergednormalisedtouch[95], mergednormalisedtouch[96], mergednormalisedtouch[97], mergednormalisedtouch[98],
                mergednormalisedtouch[99], mergednormalisedtouch[100], mergednormalisedtouch[101], mergednormalisedtouch[102], mergednormalisedtouch[103], mergednormalisedtouch[104], mergednormalisedtouch[105], mergednormalisedtouch[106],mergednormalisedtouch[107],
                mergednormalisedtouch[108],mergednormalisedtouch[109],mergednormalisedtouch[110], mergednormalisedtouch[111], mergednormalisedtouch[112], mergednormalisedtouch[113],
                mergednormalisedtouch[114], mergednormalisedtouch[115], mergednormalisedtouch[116], mergednormalisedtouch[117], mergednormalisedtouch[118], mergednormalisedtouch[119]);
                oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/touch/discrete");
                lo_send(osc2, oscNamespace.c_str(), "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii", mergeddiscretetouch[0], mergeddiscretetouch[1],mergeddiscretetouch[2],
                mergeddiscretetouch[3],mergeddiscretetouch[4],mergeddiscretetouch[5], mergeddiscretetouch[6], mergeddiscretetouch[7], mergeddiscretetouch[8],
                mergeddiscretetouch[9], mergeddiscretetouch[10], mergeddiscretetouch[11], mergeddiscretetouch[12], mergeddiscretetouch[13], mergeddiscretetouch[14], mergeddiscretetouch[15], mergeddiscretetouch[16],mergeddiscretetouch[17],
                mergeddiscretetouch[18],mergeddiscretetouch[19],mergeddiscretetouch[20], mergeddiscretetouch[21], mergeddiscretetouch[22], mergeddiscretetouch[23],
                mergeddiscretetouch[24], mergeddiscretetouch[25], mergeddiscretetouch[26], mergeddiscretetouch[27], mergeddiscretetouch[28], mergeddiscretetouch[29], mergeddiscretetouch[30], mergeddiscretetouch[31], 
                mergeddiscretetouch[32], mergeddiscretetouch[33], mergeddiscretetouch[34], mergeddiscretetouch[35], mergeddiscretetouch[36], mergeddiscretetouch[37], mergeddiscretetouch[38], mergeddiscretetouch[39], 
                mergeddiscretetouch[40], mergeddiscretetouch[41], mergeddiscretetouch[42], mergeddiscretetouch[43], mergeddiscretetouch[44],mergeddiscretetouch[45],mergeddiscretetouch[46], mergeddiscretetouch[47], 
                mergeddiscretetouch[48], mergeddiscretetouch[49], mergeddiscretetouch[50],mergeddiscretetouch[51], mergeddiscretetouch[52],mergeddiscretetouch[53],
                mergeddiscretetouch[54], mergeddiscretetouch[55], mergeddiscretetouch[56], mergeddiscretetouch[57], mergeddiscretetouch[58], mergeddiscretetouch[59], mergeddiscretetouch[60], mergeddiscretetouch[61],mergeddiscretetouch[62],
                mergeddiscretetouch[63],mergeddiscretetouch[64],mergeddiscretetouch[65], mergeddiscretetouch[66], mergeddiscretetouch[67], mergeddiscretetouch[68],
                mergeddiscretetouch[69], mergeddiscretetouch[70], mergeddiscretetouch[71], mergeddiscretetouch[72], mergeddiscretetouch[73], mergeddiscretetouch[74], mergeddiscretetouch[75], mergeddiscretetouch[76],mergeddiscretetouch[77],
                mergeddiscretetouch[78],mergeddiscretetouch[79],mergeddiscretetouch[80], mergeddiscretetouch[81], mergeddiscretetouch[82], mergeddiscretetouch[83],
                mergeddiscretetouch[84], mergeddiscretetouch[85], mergeddiscretetouch[86], mergeddiscretetouch[87], mergeddiscretetouch[88], mergeddiscretetouch[89], mergeddiscretetouch[90], mergeddiscretetouch[91],mergeddiscretetouch[92],
                mergeddiscretetouch[93],mergeddiscretetouch[94],mergeddiscretetouch[95], mergeddiscretetouch[96], mergeddiscretetouch[97], mergeddiscretetouch[98],
                mergeddiscretetouch[99], mergeddiscretetouch[100], mergeddiscretetouch[101], mergeddiscretetouch[102], mergeddiscretetouch[103], mergeddiscretetouch[104], mergeddiscretetouch[105], mergeddiscretetouch[106],mergeddiscretetouch[107],
                mergeddiscretetouch[108],mergeddiscretetouch[109],mergeddiscretetouch[110], mergeddiscretetouch[111], mergeddiscretetouch[112], mergeddiscretetouch[113],
                mergeddiscretetouch[114], mergeddiscretetouch[115], mergeddiscretetouch[116], mergeddiscretetouch[117], mergeddiscretetouch[118], mergeddiscretetouch[119]);
            }

            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/touch/all");
            lo_send(osc2, oscNamespace.c_str(), "f", gestures.touchAll);
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/touch/top");
            lo_send(osc2, oscNamespace.c_str(), "f", gestures.touchTop);
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/touch/middle");
            lo_send(osc2, oscNamespace.c_str(), "f", gestures.touchMiddle);
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/touch/bottom");
            lo_send(osc2, oscNamespace.c_str(), "f", gestures.touchBottom);
        }
        if (event.mimu) {
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "raw/accl");
            lo_send(osc2, oscNamespace.c_str(), "fff", sensors.accl[0], sensors.accl[1], sensors.accl[2]);
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "raw/gyro");
            lo_send(osc2, oscNamespace.c_str(), "fff", sensors.gyro[0], sensors.gyro[1], sensors.gyro[2]);
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "raw/magn");
            lo_send(osc2, oscNamespace.c_str(), "fff", sensors.magn[0], sensors.magn[1], sensors.magn[2]);
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "orientation");
            lo_send(osc2, oscNamespace.c_str(), "ffff", sensors.quat[0], sensors.quat[1], sensors.quat[2], sensors.quat[3]);
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "ypr");
            lo_send(osc2, oscNamespace.c_str(), "fff", sensors.ypr[0], sensors.ypr[1], sensors.ypr[2]);        
            // Reset mimu event
            event.mimu = false;        
        }
        if (event.brush) {
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/brush");
            lo_send(osc2, oscNamespace.c_str(), "f", sensors.brush);
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/multibrush");
            lo_send(osc2, oscNamespace.c_str(), "fff", sensors.multibrush[0], sensors.multibrush[1], sensors.multibrush[2]);
        }
        if (event.rub) {
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/rub");
            lo_send(osc2, oscNamespace.c_str(), "f", sensors.rub);
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/multirub");
            lo_send(osc2, oscNamespace.c_str(), "fff", sensors.multirub[0], sensors.multirub[1], sensors.multirub[2]);
        }
        if (event.shake) {
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/shakexyz");
            lo_send(osc2, oscNamespace.c_str(), "fff", sensors.shake[0], sensors.shake[1], sensors.shake[2]);
        }
        if (event.jab) {
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/jabxyz");
            lo_send(osc2, oscNamespace.c_str(), "fff", sensors.jab[0], sensors.jab[1], sensors.jab[2]);
        }
        if (event.count) {
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/button/count");
            lo_send(osc2, oscNamespace.c_str(), "i", sensors.count);
        }
        if (event.tap) {
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/button/tap");
            lo_send(osc2, oscNamespace.c_str(), "i", sensors.tap);
        }
        if (event.dtap) {
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/button/dtap");
            lo_send(osc2, oscNamespace.c_str(), "i", sensors.dtap);
        }
        if (event.ttap) {
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/button/ttap");
            lo_send(osc2, oscNamespace.c_str(), "i", sensors.ttap);
        }
        if (event.battery) {
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "battery/percentage");
            lo_send(osc2, oscNamespace.c_str(), "i", battery.percentage);
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "battery/capacity");
            lo_send(osc2, oscNamespace.c_str(), "i", battery.capacity);
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "battery/status");
            lo_send(osc2, oscNamespace.c_str(), "i", battery.status);       
        }
        if (event.current) {
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "battery/current");
            lo_send(osc2, oscNamespace.c_str(), "i", battery.current);
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "battery/tte");
            lo_send(osc2, oscNamespace.c_str(), "f", battery.TTE);
        }
        if (event.voltage) {
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "battery/voltage");
            lo_send(osc2, oscNamespace.c_str(), "f", battery.voltage);
        }    
    }
}

// Sensor callbacks
void readIMU() {
    imu.getData();

    // Save data to puara gestures
    gestures.setAccelerometerValues(imu.accl[0],
                                    imu.accl[1],
                                    imu.accl[2]);
    gestures.setGyroscopeValues(imu.gyro[0],
                                imu.gyro[1],
                                imu.gyro[2]);
    gestures.setMagnetometerValues(imu.magn[0],
                                    imu.magn[1],
                                    imu.magn[2]);

    // Convert accel from g's to meters/sec^2
    sensors.accl[0] = gestures.getAccelX() * 9.80665;
    sensors.accl[1] = gestures.getAccelY() * 9.80665;
    sensors.accl[2] = gestures.getAccelZ() * 9.80665;
    // Convert gyro from degrees/sec to radians/sec
    sensors.gyro[0] = gestures.getGyroX() * M_PI / 180;
    sensors.gyro[1] = gestures.getGyroY() * M_PI / 180;
    sensors.gyro[2] = gestures.getGyroZ() * M_PI / 180;
    // Convert mag from Gauss to uTesla
    sensors.magn[0] = gestures.getMagX();
    sensors.magn[1] = gestures.getMagY();
    sensors.magn[2] = gestures.getMagZ();

    // set imu event true
    event.mimu = true;

    // disable task
    updateIMU.disable();
}

void readTouch() {
    #ifdef touch_ENCHANTI
    touch.i2ctimer = micros();
    #endif
    // Read Touch
    touch.readTouch();
    touch.cookData();

    // Store in arrays for sensing
    for (int i = 0; i < TSTICK_SIZE; ++i) {
        mergedtouch[i] = touch.touch[i];
        mergeddiscretetouch[i] = touch.discreteTouch[i];
        mergednormalisedtouch[i] = touch.normTouch[i];
    }

    // set timer
    #ifdef touch_ENCHANTI
    unsigned long  now = micros();
    touch.polltime = now - touch.i2ctimer;
    touch.i2ctimer = now;
    #endif

    // Update touch gestures
    gestures.updateTouchArray(mergeddiscretetouch,TSTICK_SIZE);

    // Update event structure
    if (touch.newData) {
        event.touchReady = true;
        touch.newData = 0;
    } else {
        event.touchReady = false;
    }
}

void readAnalog() {
    // Update button
    gestures.updateTrigButton(button.getButton());
    
    // Read FSR
    fsr.readFsr();

    // Update array
    sensors.fsr = fsr.getValue();
    sensors.squeeze = fsr.getNormValue();

    // Update button
    if (sensors.count != gestures.getButtonCount()) {sensors.count = gestures.getButtonCount(); event.count = true; } else { event.count = false; }
    if (sensors.tap != gestures.getButtonTap()) {sensors.tap = gestures.getButtonTap(); event.tap = true; } else { event.tap = false; }
    if (sensors.dtap != gestures.getButtonDTap()) {sensors.dtap = gestures.getButtonDTap(); event.dtap = true; } else { event.dtap = false; }
    if (sensors.ttap != gestures.getButtonTTap()) {sensors.ttap = gestures.getButtonTTap(); event.ttap = true; } else { event.ttap = false; }
}

void readBattery() {
    // Read battery stats from fuel gauge
    fuelgauge.getBatteryData();
    fuelgauge.getBatteryStatus();

    // Store battery info
    battery.percentage = fuelgauge.rep_soc;
    battery.current = fuelgauge.rep_inst_current;
    battery.voltage = fuelgauge.rep_avg_voltage;
    battery.TTE = fuelgauge.rep_tte;
    battery.rsense = fuelgauge.rsense;
    battery.capacity = fuelgauge.rep_capacity;
    battery.status = fuelgauge.bat_status;

    // Save to sensors array
    if (sensors.battery != battery.percentage) {sensors.battery = battery.percentage; event.battery = true; } else { event.battery = false; }
    if (sensors.current != battery.current) {sensors.current = battery.current; event.current = true; } else { event.current = false; }
    if (sensors.voltage != battery.voltage) {sensors.voltage = battery.voltage; event.voltage = true; } else { event.voltage = false; }
}

void changeLED() {
    // Set LED - connection status and battery level
    #ifdef board_ENCHANTI_rev2
    if (puara.get_StaIsConnected()) {         // blinks when connected, cycle when disconnected
        // If connected to WiFi turn off Orange LED
        if (digitalRead(ORANGE_LED)) {
            digitalWrite(ORANGE_LED, LOW);
        }
        // Cycle LED on and Off
        led.setInterval(1000);                // RGB: 0, 128, 255 (Dodger Blue)
        led_var.color = led.blink(HIGH,20);
        digitalWrite(BLUE_LED, led_var.color);
        } else {
        // If not connected to WiFi turn off blue LED
        if (digitalRead(BLUE_LED)) {
            digitalWrite(BLUE_LED, LOW);
        }
        // Cycle LED on and Off
        led.setInterval(4000);
        led_var.color = led.cycle(led_var.color, 0, 255);
        digitalWrite(ORANGE_LED, led_var.color);
    }
    #endif
}

void updateGestures() {
    // Update Jab thresholds
    gestures.jabXThreshold = puara.getVarNumber("jabx_threshold");
    gestures.jabYThreshold = puara.getVarNumber("jaby_threshold");
    gestures.jabZThreshold = puara.getVarNumber("jabz_threshold");

    // Update inertial gestures
    gestures.updateInertialGestures();

    // Orientation quaternion
    sensors.quat[0] = gestures.getOrientationQuaternion().w;
    sensors.quat[1] = gestures.getOrientationQuaternion().x;
    sensors.quat[2] = gestures.getOrientationQuaternion().y;
    sensors.quat[3] = gestures.getOrientationQuaternion().z;
    // Yaw (heading), pitch (tilt) and roll
    sensors.ypr[0] = ((round(gestures.getYaw() * 1000)) / 1000);
    sensors.ypr[1] = ((round(gestures.getPitch() * 1000)) / 1000);
    sensors.ypr[2] = ((round(gestures.getRoll() * 1000)) / 1000);

    if (sensors.shake[0] != gestures.getShakeX() || sensors.shake[1] != gestures.getShakeY() || sensors.shake[2] != gestures.getShakeZ()) {
        sensors.shake[0] = gestures.getShakeX();
        sensors.shake[1] = gestures.getShakeY();
        sensors.shake[2] = gestures.getShakeZ();
        event.shake = true;
    } else { event.shake = false; }
    if (sensors.brush != gestures.brush || sensors.multibrush[0] != gestures.multiBrush[0]) {
        sensors.brush = gestures.brush;
        sensors.multibrush[0] = gestures.multiBrush[0];
        sensors.multibrush[1] = gestures.multiBrush[1];
        sensors.multibrush[2] = gestures.multiBrush[2];
        sensors.multibrush[3] = gestures.multiBrush[3];
        event.brush = true;
    } else { event.brush = false; }
    if (sensors.rub != gestures.rub || sensors.multirub[0] != gestures.multiRub[0]) {
        sensors.rub = gestures.rub;
        sensors.multirub[0] = gestures.multiRub[0];
        sensors.multirub[1] = gestures.multiRub[1];
        sensors.multirub[2] = gestures.multiRub[2];
        sensors.multirub[3] = gestures.multiRub[3];
        event.rub = true;
    } else { event.rub = false; }
    if (sensors.jab[0] != gestures.getJabX() || sensors.jab[1] != gestures.getJabY() || sensors.jab[2] != gestures.getJabZ()) {
        sensors.jab[0] = gestures.getJabX();
        sensors.jab[1] = gestures.getJabY();
        sensors.jab[2] = gestures.getJabZ();
        event.jab = true;
    } else { event.jab = false; }
}

#ifdef imu_ICM20948
void imu_isr() {
    updateIMU.restart();
}
#endif

// Set up multithreading
#define COMMS_CPU 0
#define SENSOR_CPU 1

// ===== rtos task handles =========================
TaskHandle_t tSensors;
TaskHandle_t tMappings;

// Mappings
void tSensorTasks(void* parameters)  {
  for(;;){
    runnerSensors.execute();
  }
}

void tMappingTasks(void* parameters) {
  for(;;){
    runnerComms.execute();
  }
}

void createCoreTasks() {
  xTaskCreatePinnedToCore(
    tSensorTasks,
    "sensors",
    8096,
    NULL,
    2,
    &tSensors,
    SENSOR_CPU);

  xTaskCreatePinnedToCore(
    tMappingTasks,   /* Task function. */
    "comms",     /* name of task. */
    10000,       /* Stack size of task */
    NULL,        /* parameter of the task */
    2,           /* priority of the task */
    &tMappings,  /* task handle */
    COMMS_CPU);
}


///////////
// setup //
///////////

void setup() {
    // Initialise LED
    pinMode(ORANGE_LED, OUTPUT);
    pinMode(BLUE_LED, OUTPUT);

    // Turn on orange LED to indicate setup
    digitalWrite(ORANGE_LED, HIGH);
    
    // Set up I2C clock
    Wire.begin(SDA_PIN, SCL_PIN);
    Wire.setClock(I2CUPDATE_FREQ); // Fast mode

    // Disable WiFi power save
    esp_wifi_set_ps(WIFI_PS_NONE);

    puara.set_version(firmware_version);

    // Start Serial Monitor
    Serial.begin(115200);
    
    // Set monitor type and start
    puara.start(Puara::JTAG_MONITOR);
    baseNamespace.append(puara.get_dmi_name());
    baseNamespace.append("/");
    oscNamespace = baseNamespace;

    std::cout << "    Initializing button configuration... ";
    if (button.initButton(pin.button)) {
        std::cout << "done" << std::endl;
    } else {
        std::cout << "initialization failed!" << std::endl;
    }
    // Initialise Button Interrupt
    attachInterrupt(pin.button, buttton_isr, CHANGE);

    std::cout << "    Initializing IMU... ";
    #ifdef imu_ICM20948
        imu.initIMU(MIMUBOARD::mimu_ICM20948);
    #endif
    #ifdef imu_LSM9DS1
        imu.initIMU(MIMUBOARD::mimu_LSM9DS1);
    #endif
    readIMU(); // get some data and save it to avoid puara-gesture crashes due to empty buffer
    std::cout << "done" << std::endl;

    // Initialise Fuel Gauge
    std::cout << "    Initializing Fuel Gauge configuration... ";
    if (fuelgauge.init(fg_config)) {
        std::cout << "done" << std::endl;
    } else {
        std::cout << "initialization failed!" << std::endl;
    }


    // Setup IMU intterupt
    pinMode(IMU_INT_PIN, INPUT_PULLUP);
    attachInterrupt(IMU_INT_PIN, imu_isr, FALLING);
    std::cout << "done" << std::endl;

    // Setup jabx,jaby and jabz thresholds
    gestures.jabXThreshold = puara.getVarNumber("jabx_threshold");
    gestures.jabYThreshold = puara.getVarNumber("jaby_threshold");
    gestures.jabZThreshold = puara.getVarNumber("jabz_threshold");

    // Calibrate IMU
    // Set acceleration zero rate
    imuParams.accel_zerog[0] = puara.getVarNumber("accel_zerog1");
    imuParams.accel_zerog[1] = puara.getVarNumber("accel_zerog2");
    imuParams.accel_zerog[2] = puara.getVarNumber("accel_zerog3");

    // Set gyroscope zero rate
    imuParams.gyro_zerorate[0] = puara.getVarNumber("gyro_zerorate1");
    imuParams.gyro_zerorate[1] = puara.getVarNumber("gyro_zerorate2");
    imuParams.gyro_zerorate[2] = puara.getVarNumber("gyro_zerorate3");

    // Set Magnetometer hard offset
    imuParams.h[0] = puara.getVarNumber("hard_offset1");
    imuParams.h[1] = puara.getVarNumber("hard_offset2");
    imuParams.h[2] = puara.getVarNumber("hard_offset3");

    // Set magnetometer soft offset
    imuParams.sx[0] = puara.getVarNumber("soft_offsetx1");
    imuParams.sx[1] = puara.getVarNumber("soft_offsetx2");
    imuParams.sx[2] = puara.getVarNumber("soft_offsetx3");

    imuParams.sy[0] = puara.getVarNumber("soft_offsety1");
    imuParams.sy[1] = puara.getVarNumber("soft_offsety2");
    imuParams.sy[2] = puara.getVarNumber("soft_offsety3");

    imuParams.sz[0] = puara.getVarNumber("soft_offsetz1");
    imuParams.sz[1] = puara.getVarNumber("soft_offsetz2");
    imuParams.sz[2] = puara.getVarNumber("soft_offsetz3");

    // Set calibration parameters
    gestures.setCalibrationParameters(imuParams);

    std::cout << "    Initializing FSR... ";
    if (fsr.initFsr(pin.fsr, std::round(puara.getVarNumber("fsr_offset")))) {
        std::cout << "done (offset value: " << fsr.getOffset() << ")" << std::endl;
    } else {
        std::cout << "initialization failed!" << std::endl;
    }

    std::cout << "    Initializing touch sensor... ";
    std::fill_n(lm.touchMin, TSTICK_SIZE, 0);
    std::fill_n(lm.touchMax, TSTICK_SIZE, 1);

    #ifdef touch_TRILL
        // Compute number of boards from TSTICK_SIZE
        float num_boards = TSTICK_SIZE / TRILL_BASETOUCHSIZE;
        int touch_noise = puara.getVarNumber("touch_noise");
        if (touch.initTouch(num_boards, touch_noise)) {
            std::cout << "done" << std::endl;
        } else {
            std::cout << "initialization failed!" << std::endl;
        }
    #endif
    #ifdef touch_ENCHANTI
        // Compute number of boards from TSTICK_SIZE
        float num_boards = TSTICK_SIZE / ENCHANTI_BASETOUCHSIZE;
        int touch_noise = puara.getVarNumber("touch_noise");
        touch.initTouch(num_boards, touch_noise);
        std::cout << "done" << std::endl;
    #endif

    std::cout << "    Initializing Liblo server/client at " << puara.getLocalPORTStr() << " ... ";
    osc1 = lo_address_new(puara.getIP1().c_str(), puara.getPORT1Str().c_str());
    osc2 = lo_address_new(puara.getIP2().c_str(), puara.getPORT2Str().c_str());
    osc_server = lo_server_thread_new(puara.getLocalPORTStr().c_str(), error);
    lo_server_thread_add_method(osc_server, NULL, NULL, generic_handler, NULL);
    lo_server_thread_start(osc_server);
    std::cout << "done" << std::endl;

    std::cout << "    Initializing Libmapper device/signals... ";
    lm_dev = mpr_dev_new(puara.get_dmi_name().c_str(), 0);
    lm.fsr = mpr_sig_new(lm_dev, MPR_DIR_OUT, "raw/fsr", 1, MPR_INT32, "un", &lm.fsrMin, &lm.fsrMax, 0, 0, 0);
    lm.accel = mpr_sig_new(lm_dev, MPR_DIR_OUT, "raw/accel", 3, MPR_FLT, "m/s^2",  &lm.accelMin, &lm.accelMax, 0, 0, 0);
    lm.gyro = mpr_sig_new(lm_dev, MPR_DIR_OUT, "raw/gyro", 3, MPR_FLT, "rad/s", &lm.gyroMin, &lm.gyroMax, 0, 0, 0);
    lm.magn = mpr_sig_new(lm_dev, MPR_DIR_OUT, "raw/mag", 3, MPR_FLT, "uTesla", &lm.magnMin, &lm.magnMax, 0, 0, 0);
    lm.quat = mpr_sig_new(lm_dev, MPR_DIR_OUT, "orientation", 4, MPR_FLT, "qt", lm.quatMin, lm.quatMax, 0, 0, 0);
    lm.ypr = mpr_sig_new(lm_dev, MPR_DIR_OUT, "ypr", 3, MPR_FLT, "fl", lm.yprMin, lm.yprMax, 0, 0, 0);
    lm.squeeze = mpr_sig_new(lm_dev, MPR_DIR_OUT, "instrument/squeeze", 1, MPR_FLT, "fl", &lm.squeezeMin, &lm.squeezeMax, 0, 0, 0);
    lm.shake = mpr_sig_new(lm_dev, MPR_DIR_OUT, "instrument/shake", 3, MPR_FLT, "fl", lm.shakeMin, lm.shakeMax, 0, 0, 0);
    lm.jab = mpr_sig_new(lm_dev, MPR_DIR_OUT, "instrument/jab", 3, MPR_FLT, "fl", lm.jabMin, lm.jabMax, 0, 0, 0);
    lm.brush = mpr_sig_new(lm_dev, MPR_DIR_OUT, "instrument/brush", 1, MPR_FLT, "un", lm.brushMin, lm.brushMax, 0, 0, 0);
    lm.rub = mpr_sig_new(lm_dev, MPR_DIR_OUT, "instrument/rub", 1, MPR_FLT, "un", lm.rubMin, lm.rubMax, 0, 0, 0);
    lm.multibrush = mpr_sig_new(lm_dev, MPR_DIR_OUT, "instrument/multibrush", 4, MPR_FLT, "un", lm.brushMin, lm.brushMax, 0, 0, 0);
    lm.multirub = mpr_sig_new(lm_dev, MPR_DIR_OUT, "instrument/multirub", 4, MPR_FLT, "un", lm.rubMin, lm.rubMax, 0, 0, 0);
    #ifdef touch_TRILL
        lm.rawtouch = mpr_sig_new(lm_dev, MPR_DIR_OUT, "raw/capsense", touch.touchSize, MPR_INT32, "un", &lm.touchMin, &lm.touchMax, 0, 0, 0);
        lm.disctouch = mpr_sig_new(lm_dev, MPR_DIR_OUT, "instrument/discretetouch", touch.touchSize, MPR_INT32, "un", &lm.touchMin, &lm.touchMax, 0, 0, 0);
    #endif
    #ifdef touch_CAPSENSE
        lm.rawtouch = mpr_sig_new(lm_dev, MPR_DIR_OUT, "raw/capsense", capsense.touchStripsSize, MPR_INT32, "un", &lm.touchMin, &lm.touchMax, 0, 0, 0);
    #endif
    lm.count = mpr_sig_new(lm_dev, MPR_DIR_OUT, "instrument/count", 1, MPR_INT32, "un", &lm.countMin, &lm.countMax, 0, 0, 0);
    lm.tap = mpr_sig_new(lm_dev, MPR_DIR_OUT, "instrument/tap", 1, MPR_INT32, "un", &lm.tapMin, &lm.tapMax, 0, 0, 0);
    lm.ttap = mpr_sig_new(lm_dev, MPR_DIR_OUT, "instrument/triple tap", 1, MPR_INT32, "un", &lm.tapMin, &lm.tapMax, 0, 0, 0);
    lm.dtap = mpr_sig_new(lm_dev, MPR_DIR_OUT, "instrument/double tap", 1, MPR_INT32, "un", &lm.tapMin, &lm.tapMax, 0, 0, 0);
    lm.soc = mpr_sig_new(lm_dev, MPR_DIR_OUT, "battery/percentage", 1, MPR_FLT, "%", &lm.batSOCMin, &lm.batSOCMax, 0, 0, 0);
    lm.batvolt = mpr_sig_new(lm_dev, MPR_DIR_OUT, "battery/voltage", 1, MPR_FLT, "V", &lm.batVoltMin, &lm.batVoltMax, 0, 0, 0);
    std::cout << "done" << std::endl;

    // Setting Deep sleep wake button
    rtc_gpio_pullup_en(GPIO_NUM_9);
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_9,0); // 1 = High, 0 = Low

    // Enable tasks
    #ifdef OSC2
    OSCupdate2.enable();
    #endif
    runnerSensors.enableAll();
    
    // Using Serial.print and delay to prevent interruptions
    delay(500);
    std::cout << puara.get_dmi_name().c_str() << std::endl;
    std::cout << "Edu Meneses\nMetalab - Société des Arts Technologiques (SAT)\nIDMIL - CIRMMT - McGill University" << std::endl;
    std::cout << "Firmware version: \n" << firmware_version<< "\n" << std::endl;

    // Create tasks
    // createCoreTasks();
}

//////////
// loop //
//////////

void loop() {
    runnerComms.execute();
    runnerSensors.execute();
}