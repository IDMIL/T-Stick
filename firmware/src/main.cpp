//****************************************************************************//
// T-Stick - sopranino/soprano firmware                                       //
// SAT/Metalab                                                                //
// Input Devices and Music Interaction Laboratory (IDMIL), McGill University  //
// Edu Meneses (2022) - https://www.edumeneses.com                            //
//****************************************************************************//

/* Created using the Puara template: https://github.com/Puara/puara-module-template 
 * The template contains a fully commented version for the commonly used commands 
 */


unsigned int firmware_version = 240401;
#define DEBUG
/*
Include T-Stick properties
- Go to tstick-presets.h to edit properties for your T-Stick
*/ 
#include "tstick-presets.h"

#include "Arduino.h"
#include "time.h"
// For JTAG monitor
#include "USB.h"

// For disabling power saving
#include "esp_wifi.h"

#include <puara.h>
#include <puara_gestures.h>
#include <mapper.h>
#include <charconv>
#include <lo/lo.h>
#include <lo/lo_lowlevel.h>
#include <lo/lo_types.h>

#include <deque>
#include <cmath>
#include <algorithm>
#include <numeric>
// initializing libmapper, puara, puara-gestures, and liblo client
mpr_dev lm_dev = 0;
Puara puara;
PuaraGestures gestures;
lo_address osc1;
lo_address osc2;
std::string baseNamespace = "/";
std::string oscNamespace;
bool use_osc1 = false;
bool use_osc2 = false;

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
    int led;
    int battery;
    int fsr;
    int button;
};

TSTICK_Pin pin{ LED_PIN, 35, FSR_PIN, BUTTON_PIN };

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
    bool tte = false;
} event;


//////////////////////////////////
// Battery struct and functions //
//////////////////////////////////
struct BatteryData {
    float percentage = 0;
    float voltage = 0;
    float current = 0;
    float TTE = 0;
    bool status = false; // is there a battery
    uint16_t rsense = 0;
    float capacity = 0;
    uint16_t designcap = 0;
    float value;
    unsigned long timer = 0;
    int interval = 5000; // in ms (1/f)
    int queueAmount = 10; // # of values stored
    std::deque<int> filterArray; // store last values
} battery;

// read battery level (based on https://www.youtube.com/watch?v=yZjpYmWVLh8&feature=youtu.be&t=88) 
void analogBatteryRead() {
    #ifdef ARDUINO_LOLIN_D32_PRO
        battery.value = analogRead(pin.battery) / 4096.0 * 7.445;
    #elif defined(ARDUINO_TINYPICO)
        battery.value = tinypico.GetBatteryVoltage();
    #endif
    battery.percentage = static_cast<int>((battery.value - 2.9) * 100 / (4.15 - 2.9));
    if (battery.percentage > 100)
        battery.percentage = 100;
    if (battery.percentage < 0)
        battery.percentage = 0;
}

void batteryFilter() {
    battery.filterArray.push_back(battery.percentage);
    if(battery.filterArray.size() > battery.queueAmount) {
        battery.filterArray.pop_front();
    }
    battery.percentage = 0;
    for (int i=0; i<battery.filterArray.size(); i++) {
        battery.percentage += battery.filterArray.at(i);
    }
    battery.percentage /= battery.filterArray.size();
}

///////////////////////////////////
// Custom Interrupt Routines     //
///////////////////////////////////
void buttton_isr() {
    button.readButton();
}

void imu_isr() {
    event.mimu = true;
}

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

void osc_bundle_add_int(lo_bundle puara_bundle,const char *path, int value) {
    int ret = 0;
    oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), path);
    lo_message tmp_osc = lo_message_new();
    ret = lo_message_add_int32(tmp_osc, value);
    if (ret < 0) {
        lo_message_free(tmp_osc);
        return;
    }
    ret = lo_bundle_add_message(puara_bundle, oscNamespace.c_str(), tmp_osc);
    if(ret < 0) {
        std::cout << "error adding message to bundle" << std::endl;
    }
}
void osc_bundle_add_float(lo_bundle puara_bundle,const char *path, float value) {
    int ret = 0;
    oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), path);
    lo_message tmp_osc = lo_message_new();
    ret = lo_message_add_float(tmp_osc, value);
    if (ret < 0) {
        lo_message_free(tmp_osc);
        return;
    }
    ret = lo_bundle_add_message(puara_bundle, oscNamespace.c_str(), tmp_osc);
    if(ret < 0) {
        std::cout << "error adding message to bundle" << std::endl;
    }
}
void osc_bundle_add_int_array(lo_bundle puara_bundle,const char *path, int size, int *value) {
    oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), path);
    lo_message tmp_osc = lo_message_new();
    for (int i = 0; i < size; i++) {
        lo_message_add_int32(tmp_osc, value[i]);
    }
    lo_bundle_add_message(puara_bundle, oscNamespace.c_str(), tmp_osc);
}

void osc_bundle_add_float_array(lo_bundle puara_bundle,const char *path, int size,  float *value) {
    oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), path);
    lo_message tmp_osc = lo_message_new();
    for (int i = 0; i < size; i++) {
        lo_message_add_float(tmp_osc, value[i]);
    }
    lo_bundle_add_message(puara_bundle, oscNamespace.c_str(), tmp_osc);
}
////////////////////////////////
// sensors and libmapper data //
////////////////////////////////

struct Lm { 
    mpr_sig fsr = 0;  // FSR signals
    int fsrMax = 4095;
    int fsrMin = 0;
    mpr_sig squeeze = 0;
    float squeezeMax = 1.0f;
    float squeezeMin = 0.0f;
    mpr_sig accel = 0; // IMU signals
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
    mpr_sig touchAll = 0; // Touch signals
    mpr_sig touchTop = 0;
    mpr_sig touchMiddle = 0;
    mpr_sig touchBottom = 0;
    float touchgestureMax = 1.0f;
    float touchgestureMin = 0.0f;
    mpr_sig brush = 0;
    mpr_sig multibrush = 0;
    float brushMax[4] = {50, 50, 50, 50};
    float brushMin[4] = {-50, -50, -50, -50};
    mpr_sig rub = 0;
    mpr_sig multirub = 0;
    float rubMax[4] = {5, 5, 5, 5};
    float rubMin[4] = {0, 0, 0, 0};
    mpr_sig rawtouch = 0;
    int touchMax[TSTICK_SIZE]; // Initialized in setup()
    int touchMin[TSTICK_SIZE];
    mpr_sig count = 0; // Button Signals
    int countMax = 100;
    int countMin = 0;
    mpr_sig tap = 0;
    mpr_sig ttap = 0;
    mpr_sig dtap = 0;
    int tapMax = 1;
    int tapMin = 0;
    mpr_sig soc = 0; // Battery signals
    int batSOCMax = 100;
    int batSOCMin = 0;
    mpr_sig batvolt = 0;
    float batVoltMax = 4.2f;
    float batVoltMin = 0.0f;
    mpr_sig batcurr = 0;
    float batCurrMax = 2000.0f;
    float batCurrMin = -2000.0f;
    mpr_sig battte = 0;
    float battteMax = 102.0f;
    float battteMin = 0.0f;
    mpr_sig counter = 0;
    int counterMax = 80000;
    int counterMin = 0;
    mpr_sig looptime = 0;
    int timeMax = 1000000;
    int timeMin = 0;
} lm;
int use_libmapper = 0;

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
    float battery;
    float current;
    float voltage;
    float tte;
    float touchAll;         
    float touchTop;         
    float touchMiddle;      
    float touchBottom;      
    int mergedtouch[TSTICK_SIZE];
    int mergeddiscretetouch[TSTICK_SIZE];
    int counter;
    int looptime;
} sensors;

// Debug code
// Timers
uint32_t start = 0;
uint32_t end = 0;
uint32_t time_taken = 0;

// task callbacks
// Comms tasks
void updateLibmapper();
void updateOSC();
void updateOSC_bundle(lo_bundle puara_bundle);

// Sensor callbacks
void readIMU();
void readTouch();
void readAnalog();
void readBattery();
void changeLED();
void updateMIMU();

// Define callbacks
///// Comms
void updateLibmapper() {
    mpr_dev_poll(lm_dev, 0);
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
    mpr_sig_set_value(lm.soc, 0, 1, MPR_INT32, &sensors.battery);
    mpr_sig_set_value(lm.batvolt, 0, 1, MPR_FLT, &sensors.voltage);
    mpr_sig_set_value(lm.batcurr, 0, 1, MPR_FLT, &sensors.current);
    mpr_sig_set_value(lm.battte, 0, 1, MPR_FLT, &sensors.tte);
    mpr_sig_set_value(lm.rawtouch, 0, TSTICK_SIZE, MPR_INT32, &sensors.mergedtouch);
    mpr_sig_set_value(lm.touchAll, 0, 1, MPR_FLT, &sensors.touchAll);
    mpr_sig_set_value(lm.touchTop, 0, 1, MPR_FLT, &sensors.touchTop);
    mpr_sig_set_value(lm.touchMiddle, 0, 1, MPR_FLT, &sensors.touchMiddle);
    mpr_sig_set_value(lm.touchBottom, 0, 1, MPR_FLT, &sensors.touchBottom);
    mpr_sig_set_value(lm.counter, 0, 1, MPR_INT32, &sensors.counter);
    mpr_sig_set_value(lm.looptime, 0, 1, MPR_INT32, &sensors.looptime);
    mpr_dev_update_maps(lm_dev);
}

void updateOSC() {
    // Create a bundle and send it to both IP addresses
    if (use_osc1 || use_osc2) {
        lo_bundle bundle = lo_bundle_new(LO_TT_IMMEDIATE);
        if (!bundle) {
            return;
        }
        updateOSC_bundle(bundle);

        if (use_osc1) {
            lo_send_bundle(osc1, bundle);
        }
        if (use_osc2) {
            lo_send_bundle(osc2, bundle);
        }

        // free memory from bundle
        lo_bundle_free_recursive(bundle);
    }
}

void updateOSC_bundle(lo_bundle bundle) {
    // Continuously send FSR data
    osc_bundle_add_int(bundle, "raw/fsr", sensors.fsr);
    osc_bundle_add_float(bundle, "instrument/squeeze", sensors.squeeze);

    //Send touch data
    osc_bundle_add_float(bundle, "instrument/touch/all", sensors.touchAll);
    osc_bundle_add_float(bundle, "instrument/touch/top", sensors.touchTop);
    osc_bundle_add_float(bundle, "instrument/touch/middle", sensors.touchMiddle);
    osc_bundle_add_float(bundle, "instrument/touch/bottom", sensors.touchBottom);
    osc_bundle_add_int_array(bundle, "instrument/touch/discrete", TSTICK_SIZE, sensors.mergeddiscretetouch);
    osc_bundle_add_int_array(bundle, "raw/capsense", TSTICK_SIZE, sensors.mergedtouch);
    
    // Touch gestures
    if (event.brush) {
        osc_bundle_add_float(bundle, "instrument/brush", sensors.brush);
        osc_bundle_add_float_array(bundle, "instrument/multibrush", 4, sensors.multibrush);
    }
    if (event.rub) {
        osc_bundle_add_float(bundle, "instrument/rub", sensors.rub);
        osc_bundle_add_float_array(bundle, "instrument/multirub", 4, sensors.multirub);
    }

    
    // MIMU data
    osc_bundle_add_float_array(bundle, "raw/accl", 3, sensors.accl);
    osc_bundle_add_float_array(bundle, "raw/gyro", 3, sensors.gyro);
    osc_bundle_add_float_array(bundle, "raw/magn", 3, sensors.magn);
    osc_bundle_add_float_array(bundle, "orientation", 4, sensors.quat);
    osc_bundle_add_float_array(bundle, "ypr", 3, sensors.ypr); 

    // Inertial gestures
    if (event.shake) {
        osc_bundle_add_float_array(bundle, "instrument/shakexyz", 3, sensors.shake);
    }
    if (event.jab) {
        osc_bundle_add_float_array(bundle, "instrument/jabxyz", 3, sensors.jab);
    }
    // Button Gestures
    if (event.count) {
        osc_bundle_add_int(bundle, "instrument/button/count", sensors.count);
    }
    if (event.tap) {
        osc_bundle_add_int(bundle, "instrument/button/tap", sensors.tap);
    }
    if (event.dtap) {
        osc_bundle_add_int(bundle, "instrument/button/dtap", sensors.dtap);
    }
    if (event.ttap) {
        osc_bundle_add_int(bundle, "instrument/button/ttap", sensors.ttap);
    }

    // Battery Data
    if (event.battery) {
        osc_bundle_add_float(bundle, "battery/percentage", sensors.battery);
    }
    if (event.current) {
        osc_bundle_add_float(bundle, "battery/current", sensors.current);
    }
    if (event.tte) {
        osc_bundle_add_float(bundle, "battery/timetoempty", sensors.tte);
    }
    if (event.voltage) {
        osc_bundle_add_float(bundle, "battery/voltage", sensors.voltage);  
    }

    // Add counter
    osc_bundle_add_int(bundle, "test/counter", sensors.counter);
    osc_bundle_add_int(bundle, "test/looptime", sensors.looptime);
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
}

void readTouch() {
    // Read Touch
    touch.readTouch();
    touch.cookData();

    // Store in arrays for sensing
    for (int i = 0; i < TSTICK_SIZE; ++i) {
        sensors.mergedtouch[i] = touch.touch[i];
        sensors.mergeddiscretetouch[i] = touch.discreteTouch[i];
    }

    // Update touch gestures
    gestures.updateTouchArray(sensors.mergeddiscretetouch,TSTICK_SIZE);
    sensors.touchAll = gestures.touchAll;
    sensors.touchTop = gestures.touchTop;
    sensors.touchMiddle = gestures.touchMiddle;
    sensors.touchBottom = gestures.touchBottom;

    // Update event structure
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
    
    if (touch.newData || event.touchReady) {
        event.touchReady = true;
        touch.newData = 0;
    }
}
    

void readAnalog() {
    // Update button
    gestures.updateTrigButton(button.getButton());
    
    // go to deep sleep if double press button
    if (gestures.getButtonDTap()){
        std::cout << "\nEntering deep sleep.\n\nGoodbye!\n" << std::endl;
        imu.sleep();

        #ifdef LDO2
            digitalWrite(LDO_PIN, LOW); // disable second LDO
        #endif
        delay(1000);
        esp_deep_sleep_start();
    }

    // Read FSR
    fsr.readFsr();

    // Update array
    sensors.fsr = fsr.getValue();
    sensors.squeeze = fsr.getNormValue();

    // Update LED
    changeLED();

    // Update button
    if (sensors.count != gestures.getButtonCount()) {sensors.count = gestures.getButtonCount(); event.count = true; } else { event.count = false; }
    if (sensors.tap != gestures.getButtonTap()) {sensors.tap = gestures.getButtonTap(); event.tap = true; } else { event.tap = false; }
    if (sensors.dtap != gestures.getButtonDTap()) {sensors.dtap = gestures.getButtonDTap(); event.dtap = true; } else { event.dtap = false; }
    if (sensors.ttap != gestures.getButtonTTap()) {sensors.ttap = gestures.getButtonTTap(); event.ttap = true; } else { event.ttap = false; }
}

void readBattery() {
    #ifdef fg_MAX17055
    // Read battery stats from fuel gauge
    fuelgauge.getBatteryData();

    // Store battery info
    battery.percentage = fuelgauge.rep_soc;
    battery.current = fuelgauge.rep_inst_current;
    battery.voltage = fuelgauge.rep_avg_voltage;
    battery.TTE = fuelgauge.rep_tte;
    battery.rsense = fuelgauge.rsense;
    battery.capacity = fuelgauge.rep_capacity;
    battery.status = fuelgauge.bat_status;
    #elif defined(fg_NONE)
        analogBatteryRead();
        batteryFilter();
    #endif

    // Save to sensors array
    if (sensors.battery != battery.percentage) {sensors.battery = battery.percentage; event.battery = true; } else { event.battery = false; }
    if (sensors.current != battery.current) {sensors.current = battery.current; event.current = true; } else { event.current = false; }
    if (sensors.tte != battery.TTE) {sensors.tte = battery.TTE; event.tte = true; } else { event.tte = false; }
    if (sensors.voltage != battery.voltage) {sensors.voltage = battery.voltage; event.voltage = true; } else { event.voltage = false; }

    // // Send battery data always (for debugging)
    #ifdef DEBUG
        event.battery = true;
        event.voltage = true;
        event.current = true;
        event.tte = true;
    #endif
}

void changeLED() {
    // Set LED - connection status and battery level
    #if defined(board_ENCHANTI_rev2) || defined(board_ENCHANTI_rev3)
    if (puara.get_StaIsConnected()) {         // blinks when connected, cycle when disconnected
        // If connected to WiFi turn off Orange LED
        if (digitalRead(ORANGE_LED)) {
            digitalWrite(ORANGE_LED, LOW);
        }
        led.setInterval(1000);
        led_var.ledValue = led.blink(255, 40);
        ledcWrite(0, led_var.ledValue);
    } else {
        // If not connected to WiFi turn off blue LED
        ledcWrite(0, 0);
        // Cycle LED on and Off
        digitalWrite(ORANGE_LED, HIGH);
    }
    #endif
    // Set LED - connection status and battery level
    #ifdef ARDUINO_LOLIN_D32_PRO
        if (battery.percentage < 10) {        // low battery - flickering
        led.setInterval(75);
        led_var.ledValue = led.blink(255, 50);
        ledcWrite(0, led_var.ledValue);
        } else {
            if (puara.get_StaIsConnected()) { // blinks when connected, cycle when disconnected
                led.setInterval(1000);
                led_var.ledValue = led.blink(255, 40);
                ledcWrite(0, led_var.ledValue);
            } else {
                led.setInterval(4000);
                led_var.ledValue = led.cycle(led_var.ledValue, 0, 255);
                ledcWrite(0, led_var.ledValue);
            }
        }
    #elif defined(ARDUINO_TINYPICO)
        if (battery.percentage < 10) {                // low battery (red)
            led.setInterval(20);
            led_var.color = led.blink(255, 20);
            tinypico.DotStar_SetPixelColor(led_var.color, 0, 0);
        } else {
            if (puara.get_StaIsConnected()) {         // blinks when connected, cycle when disconnected
                led.setInterval(1000);                // RGB: 0, 128, 255 (Dodger Blue)
                led_var.color = led.blink(255,20);
                tinypico.DotStar_SetPixelColor(0, uint8_t(led_var.color/2), led_var.color);
            } else {
                led.setInterval(4000);
                led_var.color = led.cycle(led_var.color, 0, 255);
                tinypico.DotStar_SetPixelColor(0, uint8_t(led_var.color/2), led_var.color);
            }
        }
    #endif 

}

void updateMIMU() {
    // Get IMU data
    if (event.mimu || TSTICK_IMU == MIMUBOARD::mimu_LSM9DS1) {
        readIMU();
        event.mimu = false;
        imu.clearInterrupt();
    }
    
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

    // Send data if event is true
    if (sensors.shake[0] != gestures.getShakeX() || sensors.shake[1] != gestures.getShakeY() || sensors.shake[2] != gestures.getShakeZ()) {
        sensors.shake[0] = gestures.getShakeX();
        sensors.shake[1] = gestures.getShakeY();
        sensors.shake[2] = gestures.getShakeZ();
        event.shake = true;
    } else { event.shake = false; }
    if (sensors.jab[0] != gestures.getJabX() || sensors.jab[1] != gestures.getJabY() || sensors.jab[2] != gestures.getJabZ()) {
        sensors.jab[0] = gestures.getJabX();
        sensors.jab[1] = gestures.getJabY();
        sensors.jab[2] = gestures.getJabZ();
        event.jab = true;
    } else { event.jab = false; }
}

///////////
// setup //
///////////

void setup() {
    // Set CPU Frequency to max
    setCpuFrequencyMhz(240);

    // Enable LDO2
    #ifdef LDO2
        pinMode(LDO_PIN, OUTPUT);
        digitalWrite(LDO_PIN, HIGH);
    #endif

    #ifdef ORANGE_LED
        // Turn on orange LED to indicate setup (EnchantiS3 Boards)
        pinMode(ORANGE_LED, OUTPUT);
        digitalWrite(ORANGE_LED, HIGH);
    #endif

    // Set up LEDs
    #ifdef INDICATOR_LED
      ledcSetup(0, 5000, 8);
      ledcAttachPin(pin.led, 0);
    #endif

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

    // Setting Deep sleep wake button
    rtc_gpio_pullup_en(SLEEP_PIN);
    esp_sleep_enable_ext0_wakeup(SLEEP_PIN,0); // 1 = High, 0 = Low

    std::cout << "    Initializing IMU... ";
    imu.initIMU(TSTICK_IMU);

    // If the IMU interrupt pin is specified, setup the interrupt
    #ifdef IMU_INT_PIN
    if ((TSTICK_IMU == MIMUBOARD::mimu_ICM20948)) {
        // Setup interrupt
        pinMode(IMU_INT_PIN, INPUT_PULLUP);
        attachInterrupt(IMU_INT_PIN, imu_isr, CHANGE);
    }
    #endif

    readIMU(); // get some data and save it to avoid puara-gesture crashes due to empty buffer
    std::cout << "done" << std::endl;

    // Initialise Fuel Gauge
    #ifdef fg_MAX17055
    std::cout << "    Initializing Fuel Gauge configuration... ";
    // Get variables from settings
    fg_config.designcap = puara.getVarNumber("battery_size_mah");
    if (fuelgauge.init(fg_config)) {
        std::cout << "done" << std::endl;
    } else {
        std::cout << "initialization failed!" << std::endl;
    }
    std::cout << "done" << std::endl;
    #endif

    // Setup jabx,jaby and jabz thresholds
    gestures.jabXThreshold = puara.getVarNumber("jab_threshold");
    gestures.jabYThreshold = puara.getVarNumber("jab_threshold");
    gestures.jabZThreshold = puara.getVarNumber("jab_threshold");

    // Calibrate IMU
    // Set acceleration zero rate
    imuParams.accel_zerog[0] = ACCELZEROGX;
    imuParams.accel_zerog[1] = ACCELZEROGY;
    imuParams.accel_zerog[2] = ACCELZEROGZ;

    // Set gyroscope zero rate
    imuParams.gyro_zerorate[0] = GYROZEROX;
    imuParams.gyro_zerorate[1] = GYROZEROY;
    imuParams.gyro_zerorate[2] = GYROZEROZ;

    // Set Magnetometer hard offset
    imuParams.h[0] = HARDOFFSETX;
    imuParams.h[1] = HARDOFFSETY;
    imuParams.h[2] = HARDOFFSETZ;

    // Set magnetometer soft offset
    imuParams.sx[0] = SOFTOFFSETX1;
    imuParams.sx[1] = SOFTOFFSETX2;
    imuParams.sx[2] = SOFTOFFSETX3;

    imuParams.sy[0] = SOFTOFFSETY1;
    imuParams.sy[1] = SOFTOFFSETY2;
    imuParams.sy[2] = SOFTOFFSETY3;

    imuParams.sz[0] = SOFTOFFSETZ1;
    imuParams.sz[1] = SOFTOFFSETZ2;
    imuParams.sz[2] = SOFTOFFSETZ3;

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
    std::fill_n(lm.touchMax, TSTICK_SIZE, TOUCH_MAX);

    // update the touch noise threshold from puara variables
    tstick_touchconfig.touch_threshold = puara.getVarNumber("touch_noise");
    if (touch.initTouch(tstick_touchconfig)) {
        std::cout << "done" << std::endl;
    } else {
        std::cout << "initialization failed!" << std::endl;
    }

    // Only do rest of setup if the T-Stick is connected to WiFi
    if (puara.get_StaIsConnected()) {
        // Only check at start up
        std::cout << "    Initializing Liblo server/client at " << puara.getLocalPORTStr() << " ... ";
        if (puara.IP1_ready()) {
            std::cout << "    Initialising IP1  ... ";
            osc1 = lo_address_new(puara.getIP1().c_str(), puara.getPORT1Str().c_str());
            use_osc1 = true;
        }
        if (puara.IP2_ready()) {
            std::cout << "    Initialising IP2  ... ";
            osc2 = lo_address_new(puara.getIP2().c_str(), puara.getPORT2Str().c_str());
            use_osc2 = true;
        }
        osc_server = lo_server_thread_new(puara.getLocalPORTStr().c_str(), error);
        lo_server_thread_add_method(osc_server, NULL, NULL, generic_handler, NULL);
        lo_server_thread_start(osc_server);
        std::cout << "done" << std::endl;

        use_libmapper = puara.getVarNumber("enable_libmapper");
        if (use_libmapper) {
            std::cout << "    Initializing Libmapper device/signals... ";
            lm_dev = mpr_dev_new(puara.get_dmi_name().c_str(), 0);
            // FSR Signals
            lm.fsr = mpr_sig_new(lm_dev, MPR_DIR_OUT, "raw/fsr", 1, MPR_INT32, "un", &lm.fsrMin, &lm.fsrMax, 0, 0, 0);
            lm.squeeze = mpr_sig_new(lm_dev, MPR_DIR_OUT, "instrument/squeeze", 1, MPR_FLT, "fl", &lm.squeezeMin, &lm.squeezeMax, 0, 0, 0);
        
            // IMU Signals
            lm.accel = mpr_sig_new(lm_dev, MPR_DIR_OUT, "raw/accel", 3, MPR_FLT, "m/s^2",  &lm.accelMin, &lm.accelMax, 0, 0, 0);
            lm.gyro = mpr_sig_new(lm_dev, MPR_DIR_OUT, "raw/gyro", 3, MPR_FLT, "rad/s", &lm.gyroMin, &lm.gyroMax, 0, 0, 0);
            lm.magn = mpr_sig_new(lm_dev, MPR_DIR_OUT, "raw/mag", 3, MPR_FLT, "uTesla", &lm.magnMin, &lm.magnMax, 0, 0, 0);
            lm.quat = mpr_sig_new(lm_dev, MPR_DIR_OUT, "orientation", 4, MPR_FLT, "qt", &lm.quatMin, &lm.quatMax, 0, 0, 0);
            lm.ypr = mpr_sig_new(lm_dev, MPR_DIR_OUT, "ypr", 3, MPR_FLT, "fl", &lm.yprMin, &lm.yprMax, 0, 0, 0);
            lm.shake = mpr_sig_new(lm_dev, MPR_DIR_OUT, "instrument/shake", 3, MPR_FLT, "fl", &lm.shakeMin, &lm.shakeMax, 0, 0, 0);
            lm.jab = mpr_sig_new(lm_dev, MPR_DIR_OUT, "instrument/jab", 3, MPR_FLT, "fl", &lm.jabMin, &lm.jabMax, 0, 0, 0);
            
            // Touch signals
            lm.touchAll = mpr_sig_new(lm_dev, MPR_DIR_OUT, "instrument/touch/all", 1, MPR_FLT, "un", &lm.touchgestureMin, &lm.touchgestureMax, 0, 0, 0);
            lm.touchTop = mpr_sig_new(lm_dev, MPR_DIR_OUT, "instrument/touch/top", 1, MPR_FLT, "un", &lm.touchgestureMin, &lm.touchgestureMax, 0, 0, 0);
            lm.touchMiddle = mpr_sig_new(lm_dev, MPR_DIR_OUT, "instrument/touch/middle", 1, MPR_FLT, "un", &lm.touchgestureMin, &lm.touchgestureMax, 0, 0, 0);
            lm.touchBottom = mpr_sig_new(lm_dev, MPR_DIR_OUT, "instrument/touch/bottom", 1, MPR_FLT, "un", &lm.touchgestureMin, &lm.touchgestureMax, 0, 0, 0);
            lm.brush = mpr_sig_new(lm_dev, MPR_DIR_OUT, "instrument/brush", 1, MPR_FLT, "un", &lm.brushMin, &lm.brushMax, 0, 0, 0);
            lm.rub = mpr_sig_new(lm_dev, MPR_DIR_OUT, "instrument/rub", 1, MPR_FLT, "un", &lm.rubMin, &lm.rubMax, 0, 0, 0);
            lm.multibrush = mpr_sig_new(lm_dev, MPR_DIR_OUT, "instrument/multibrush", 4, MPR_FLT, "un", &lm.brushMin, &lm.brushMax, 0, 0, 0);
            lm.multirub = mpr_sig_new(lm_dev, MPR_DIR_OUT, "instrument/multirub", 4, MPR_FLT, "un", &lm.rubMin, &lm.rubMax, 0, 0, 0);
            lm.rawtouch = mpr_sig_new(lm_dev, MPR_DIR_OUT, "raw/capsense", TSTICK_SIZE, MPR_INT32, "un", &lm.touchMin, &lm.touchMax, 0, 0, 0);
            
            // Button Signals
            lm.count = mpr_sig_new(lm_dev, MPR_DIR_OUT, "instrument/count", 1, MPR_INT32, "un", &lm.countMin, &lm.countMax, 0, 0, 0);
            lm.tap = mpr_sig_new(lm_dev, MPR_DIR_OUT, "instrument/tap", 1, MPR_INT32, "un", &lm.tapMin, &lm.tapMax, 0, 0, 0);
            lm.ttap = mpr_sig_new(lm_dev, MPR_DIR_OUT, "instrument/triple tap", 1, MPR_INT32, "un", &lm.tapMin, &lm.tapMax, 0, 0, 0);
            lm.dtap = mpr_sig_new(lm_dev, MPR_DIR_OUT, "instrument/double tap", 1, MPR_INT32, "un", &lm.tapMin, &lm.tapMax, 0, 0, 0);
            
            // Battery Signals
            lm.soc = mpr_sig_new(lm_dev, MPR_DIR_OUT, "battery/percentage", 1, MPR_INT32, "%", &lm.batSOCMin, &lm.batSOCMax, 0, 0, 0);
            lm.batvolt = mpr_sig_new(lm_dev, MPR_DIR_OUT, "battery/voltage", 1, MPR_FLT, "V", &lm.batVoltMin, &lm.batVoltMax, 0, 0, 0);
            lm.batcurr = mpr_sig_new(lm_dev, MPR_DIR_OUT, "battery/current", 1, MPR_FLT, "mA", &lm.batCurrMin, &lm.batCurrMax, 0, 0, 0);
            lm.battte = mpr_sig_new(lm_dev, MPR_DIR_OUT, "battery/timetoempty", 1, MPR_FLT, "h", &lm.battteMin, &lm.battteMax, 0, 0, 0);

            // Debug signals
            lm.counter = mpr_sig_new(lm_dev, MPR_DIR_OUT, "test/counter", 1, MPR_INT32, "h", &lm.counterMin, &lm.counterMax, 0, 0, 0);
            lm.looptime = mpr_sig_new(lm_dev, MPR_DIR_OUT, "test/looptime", 1, MPR_INT32, "h", &lm.timeMin, &lm.timeMax, 0, 0, 0);
            std::cout << "done" << std::endl;
        }
    }

    // Using Serial.print and delay to prevent interruptions
    delay(500);
    std::cout << puara.get_dmi_name().c_str() << std::endl;
    std::cout << "Edu Meneses\nMetalab - Société des Arts Technologiques (SAT)\nIDMIL - CIRMMT - McGill University" << std::endl;
    std::cout << "Firmware version: \n" << firmware_version<< "\n" << std::endl;
}

//////////
// loop //
//////////

void loop() {
    start = micros();
    // Read analog signals
    readAnalog();

    // Update Battery data
    if (millis() - battery.interval > battery.timer) {
      battery.timer = millis();
      readBattery();
    }

    // Get touch data
    readTouch();

    // Update MIMU data
    updateMIMU();

    // Add to counter
    #ifdef DEBUG
    sensors.counter = sensors.counter + 1;
    end = micros();
    sensors.looptime = end - start;
    #endif
    if (puara.get_StaIsConnected()) {
        // Update Libmapper and OSC
        if (use_libmapper) {
            updateLibmapper();
        }

        // Update OSC
        updateOSC();
    }
}