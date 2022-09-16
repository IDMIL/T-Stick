//****************************************************************************//
// T-Stick - sopranino/soprano firmware                                       //
// SAT/Metalab                                                                //
// Input Devices and Music Interaction Laboratory (IDMIL), McGill University  //
// Edu Meneses (2022) - https://www.edumeneses.com                            //
//****************************************************************************//

/* Created using the Puara template: https://github.com/Puara/puara-module-template 
 * The template contains a fully commented version for the commonly used commands 
 */


unsigned int firmware_version = 220916;

// set the amount of capacitive stripes for the sopranino (15) or soprano (30)
#define TSTICK_SIZE 15;

/*
  Choose the capacitive sensing board
  - Trill
  - IDMIL Capsense board 
*/
#define touch_TRILL
//#define touch_CAPSENSE


#include "Arduino.h"

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

/////////////////////
// Pin definitions //
/////////////////////

struct Pin {
    int led;     // Built In LED pin
    int battery; // To check battery level (voltage)
    int fsr;
    int button;
};

#ifdef ARDUINO_LOLIN_D32_PRO
    Pin pin{ 5, 35, 33, 15 };
#elif defined(ARDUINO_TINYPICO)
    #include "TinyPICO.h"
    Pin pin{ 5, 35, 33, 15 };
    TinyPICO tinypico = TinyPICO();
#endif

//////////////////////////////////
// Battery struct and functions //
//////////////////////////////////
  
struct BatteryData {
    unsigned int percentage = 0;
    unsigned int lastPercentage = 0;
    float value;
    unsigned long timer = 0;
    int interval = 1000; // in ms (1/f)
    int queueAmount = 10; // # of values stored
    std::deque<int> filterArray; // store last values
} battery;

// read battery level (based on https://www.youtube.com/watch?v=yZjpYmWVLh8&feature=youtu.be&t=88) 
void readBattery() {
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
// Include button function files //
///////////////////////////////////

#include "button.h"

Button button;

////////////////////////////////
// Include FSR function files //
////////////////////////////////

#include "fsr.h"

Fsr fsr;

////////////////////////////////
// Include IMU function files //
////////////////////////////////

#include "lsm9ds1.h"
Imu_LSM9DS1 imu;

//////////////////////////////////////////////
// Include Touch stuff                      //
//////////////////////////////////////////////

#ifdef touch_TRILL
  #include "touch.h"
  Touch touch;
#endif

#ifdef touch_CAPSENSE
  #include "capsense.h"
  #include "instrument_touch.h"
  Capsense capsense;
#endif

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
    float fsrMax = 4900;
    float fsrMin = 0;
    mpr_sig accel = 0;
    float accelMax = 50;
    float accelMin = -50;
    mpr_sig gyro = 0;
    float gyroMax = 25;
    float gyroMin = -25;
    mpr_sig magn = 0;
    float magnMax = 25;
    float magnMin = -25;
    mpr_sig quat = 0;
    float quatMax = 1;
    float quatMin = -1;
    mpr_sig ypr = 0;
    float yprMax = 180;
    float yprMin = -180;
    mpr_sig shake = 0;
    float shakeMax =  50;
    float shakeMin = -50;
    mpr_sig jab = 0;
    float jabMax = 50;
    float jabMin = -50;
    mpr_sig brush = 0;
    mpr_sig multibrush = 0;
    float brushMax = 50;
    float brushMin = -50;
    mpr_sig rub = 0;
    mpr_sig multirub = 0;
    float rubMax = 50;
    float rubMin = -50;
    mpr_sig touch = 0;
    int touchMax = 1;
    int touchMin = 0;
    mpr_sig count = 0;
    int countMax = 100;
    int countMin = 0;
    mpr_sig tap = 0;
    mpr_sig ttap = 0;
    mpr_sig dtap = 0;
    int tapMax = 1;
    int tapMin = 0;
    mpr_sig bat = 0;
    int batMax = 100;
    int batMin = 0;
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
    int battery;
} sensors;

struct Event {
    bool shake = false;
    bool jab = false;
    bool count = false;
    bool tap = false;
    bool dtap = false;
    bool ttap = false;
    bool fsr = false;
    bool brush = false;
    bool rub = false;
    bool battery;
} event;

///////////
// setup //
///////////

void setup() {
    #ifdef Arduino_h
        Serial.begin(115200);
    #endif

    // Disable WiFi power save
    esp_wifi_set_ps(WIFI_PS_NONE);

    puara.set_version(firmware_version);
    puara.start();
    baseNamespace.append(puara.get_dmi_name());
    baseNamespace.append("/");
    oscNamespace = baseNamespace;

    #ifdef ARDUINO_LOLIN_D32_PRO // LED init for WEMOS boards
      ledcSetup(0, 5000, 8);
      ledcAttachPin(pin.led, 0);
    #endif

    std::cout << "    Initializing button configuration... ";
    if (button.initButton(pin.button)) {
        std::cout << "done" << std::endl;
    } else {
        std::cout << "initialization failed!" << std::endl;
    }

    std::cout << "    Initializing IMU... ";
    if (imu.initIMU()) {
        std::cout << "done" << std::endl;
    } else {
        std::cout << "initialization failed!" << std::endl;
    }

    std::cout << "    Initializing FSR... ";
    if (fsr.initFsr(pin.fsr, std::round(puara.getVarNumber("fsr_offset")))) {
        std::cout << "done (offset value: " << fsr.getOffset() << ")" << std::endl;
    } else {
        std::cout << "initialization failed!" << std::endl;
    }

    std::cout << "    Initializing touch sensor... ";
    #ifdef touch_TRILL
        if (touch.initTouch()) {
            touch.touchSize = TSTICK_SIZE;
            std::cout << "done" << std::endl;
        } else {
            std::cout << "initialization failed!" << std::endl;
        }
    #endif
    #ifdef touch_CAPSENSE
        capsense.capsense_scan(); // Look for Capsense boards and return their addresses
                                // must run before initLibmapper to get # of capsense boards
            std::cout << "done" << std::endl;
    #endif

    std::cout << "    Initializing Liblo server/client... ";
    osc1 = lo_address_new(puara.getIP1().c_str(), puara.getPORT1Str().c_str());
    osc2 = lo_address_new(puara.getIP2().c_str(), puara.getPORT2Str().c_str());
    osc_server = lo_server_thread_new(puara.getLocalPORTStr().c_str(), error);
    lo_server_thread_add_method(osc_server, NULL, NULL, generic_handler, NULL);
    lo_server_thread_start(osc_server);
    std::cout << "done" << std::endl;

    std::cout << "    Initializing Libmapper device/signals... ";
    lm_dev = mpr_dev_new(puara.get_dmi_name().c_str(), 0);
    lm.fsr = mpr_sig_new(lm_dev, MPR_DIR_OUT, "raw/fsr", 1, MPR_FLT, "un", &lm.fsrMin, &lm.fsrMax, 0, 0, 0);
    lm.accel = mpr_sig_new(lm_dev, MPR_DIR_OUT, "raw/accel", 3, MPR_FLT, "m/s^2",  &lm.accelMin, &lm.accelMax, 0, 0, 0);
    lm.gyro = mpr_sig_new(lm_dev, MPR_DIR_OUT, "raw/gyro", 3, MPR_FLT, "rad/s", &lm.gyroMin, &lm.gyroMax, 0, 0, 0);
    lm.magn = mpr_sig_new(lm_dev, MPR_DIR_OUT, "raw/mag", 3, MPR_FLT, "uTesla", &lm.magnMin, &lm.magnMax, 0, 0, 0);
    lm.quat = mpr_sig_new(lm_dev, MPR_DIR_OUT, "orientation", 4, MPR_FLT, "qt", &lm.quatMin, &lm.quatMax, 0, 0, 0);
    lm.ypr = mpr_sig_new(lm_dev, MPR_DIR_OUT, "ypr", 3, MPR_FLT, "fl", &lm.yprMin, &lm.yprMax, 0, 0, 0);
    lm.shake = mpr_sig_new(lm_dev, MPR_DIR_OUT, "instrument/shake", 3, MPR_FLT, "fl", &lm.shakeMin, &lm.shakeMax, 0, 0, 0);
    lm.jab = mpr_sig_new(lm_dev, MPR_DIR_OUT, "instrument/jab", 3, MPR_FLT, "fl", &lm.jabMin, &lm.jabMax, 0, 0, 0);
    lm.brush = mpr_sig_new(lm_dev, MPR_DIR_OUT, "instrument/brush", 1, MPR_FLT, "un", &lm.brushMin, &lm.brushMax, 0, 0, 0);
    lm.rub = mpr_sig_new(lm_dev, MPR_DIR_OUT, "instrument/rub", 1, MPR_FLT, "un", &lm.rubMin, &lm.rubMax, 0, 0, 0);
    lm.multibrush = mpr_sig_new(lm_dev, MPR_DIR_OUT, "instrument/multibrush", 4, MPR_FLT, "un", &lm.brushMin, &lm.brushMax, 0, 0, 0);
    lm.multirub = mpr_sig_new(lm_dev, MPR_DIR_OUT, "instrument/multirub", 4, MPR_FLT, "un", &lm.rubMin, &lm.rubMax, 0, 0, 0);
    lm.touch = mpr_sig_new(lm_dev, MPR_DIR_OUT, "raw/capsense", touch.touchSize, MPR_INT32, "un", &lm.touchMin, &lm.touchMax, 0, 0, 0);
    lm.count = mpr_sig_new(lm_dev, MPR_DIR_OUT, "instrument/count", 1, MPR_INT32, "un", &lm.countMin, &lm.countMax, 0, 0, 0);
    lm.tap = mpr_sig_new(lm_dev, MPR_DIR_OUT, "instrument/tap", 1, MPR_INT32, "un", &lm.tapMin, &lm.tapMax, 0, 0, 0);
    lm.ttap = mpr_sig_new(lm_dev, MPR_DIR_OUT, "instrument/triple tap", 1, MPR_INT32, "un", &lm.tapMin, &lm.tapMax, 0, 0, 0);
    lm.dtap = mpr_sig_new(lm_dev, MPR_DIR_OUT, "instrument/double tap", 1, MPR_INT32, "un", &lm.tapMin, &lm.tapMax, 0, 0, 0);
    lm.bat = mpr_sig_new(lm_dev, MPR_DIR_OUT, "battery", 1, MPR_FLT, "percent", &lm.batMin, &lm.batMax, 0, 0, 0);
    std::cout << "done" << std::endl;

    // Setting Deep sleep wake button
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_15,0); // 1 = High, 0 = Low
    
    // Using Serial.print and delay to prevent interruptions
    delay(500);
    Serial.println(); 
    Serial.println(puara.get_dmi_name().c_str());
    Serial.println("Edu Meneses\nMetalab - Société des Arts Technologiques (SAT)\nIDMIL - CIRMMT - McGill University");
    Serial.print("Firmware version: "); Serial.println(firmware_version); Serial.println("\n");
}

//////////
// loop //
//////////

void loop() {

    mpr_dev_poll(lm_dev, 0);

    button.readButton();
    
    fsr.readFsr();

    // Read Touch
    #ifdef touch_TRILL
        touch.readTouch();
        touch.cookData();
        gestures.updateTouchArray(touch.touch,touch.touchSize);
    #endif
    #ifdef touch_CAPSENSE
        capsense.readCapsense();
        instrument_touch.updateTouchArray(capsense.data,capsense.touchStripsSize);
    #endif

    // read battery
    if (millis() - battery.interval > battery.timer) {
      battery.timer = millis();
      readBattery();
      batteryFilter();
    }

    // read IMU and update puara-gestures
    if (imu.dataAvailable()) {
        gestures.updateJabShake(imu.getGyroX(), imu.getGyroY(), imu.getGyroZ());
    }
    gestures.updateTrigButton(button.getButton());

    // go to deep sleep if double press button
    if (gestures.getButtonDTap()){
        std::cout << "\nEntering deep sleep.\n\nGoodbye!\n" << std::endl;
        delay(1000);
        esp_deep_sleep_start();
    }

    // Preparing arrays for libmapper signals
        sensors.fsr = fsr.getCookedValue();
        sensors.accl[0] = imu.getAccelX();
        sensors.accl[1] = imu.getAccelY();
        sensors.accl[2] = imu.getAccelZ();
        sensors.gyro[0] = imu.getGyroX();
        sensors.gyro[1] = imu.getGyroY();
        sensors.gyro[2] = imu.getGyroZ();
        sensors.magn[0] = imu.getMagX();
        sensors.magn[1] = imu.getMagY();
        sensors.magn[2] = imu.getMagZ();
        sensors.quat[0] = imu.getQuatI();
        sensors.quat[1] = imu.getQuatJ();
        sensors.quat[2] = imu.getQuatK();
        sensors.quat[3] = imu.getQuatReal();
        sensors.ypr[0] = imu.getYaw();
        sensors.ypr[1] = imu.getPitch();
        sensors.ypr[2] = imu.getRoll();
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
    if (sensors.count != gestures.getButtonCount()) {sensors.count = gestures.getButtonCount(); event.count = true; } else { event.count = false; }
    if (sensors.tap != gestures.getButtonTap()) {sensors.tap = gestures.getButtonTap(); event.tap = true; } else { event.tap = false; }
    if (sensors.dtap != gestures.getButtonDTap()) {sensors.dtap = gestures.getButtonDTap(); event.dtap = true; } else { event.dtap = false; }
    if (sensors.ttap != gestures.getButtonTTap()) {sensors.ttap = gestures.getButtonTTap(); event.ttap = true; } else { event.ttap = false; }
    if (sensors.battery != battery.percentage) {sensors.battery = battery.percentage; event.battery = true; } else { event.battery = false; }

    // updating libmapper signals
    mpr_sig_set_value(lm.fsr, 0, 1, MPR_FLT, &sensors.fsr);
    mpr_sig_set_value(lm.accel, 0, 3, MPR_FLT, &sensors.accl);
    mpr_sig_set_value(lm.gyro, 0, 3, MPR_FLT, &sensors.gyro);
    mpr_sig_set_value(lm.magn, 0, 3, MPR_FLT, &sensors.magn);
    mpr_sig_set_value(lm.quat, 0, 4, MPR_FLT, &sensors.quat);
    mpr_sig_set_value(lm.ypr, 0, 3, MPR_FLT, &sensors.ypr);
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
    mpr_sig_set_value(lm.bat, 0, 1, MPR_FLT, &sensors.battery);
    mpr_sig_set_value(lm.touch, 0, touch.touchSize, MPR_INT32, &touch.touch);

    // Sending continuous OSC messages
    if (puara.IP1_ready()) {

            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "raw/capsense");
            lo_send(osc1, oscNamespace.c_str(), "iiiiiiiiiiiiiii", touch.touch[0], touch.touch[1],touch.touch[2],
                    touch.touch[3],touch.touch[4],touch.touch[5], touch.touch[6], touch.touch[7], touch.touch[8],
                    touch.touch[9], touch.touch[10], touch.touch[11], touch.touch[12], touch.touch[13], touch.touch[14]
            );
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "raw/fsr");
            lo_send(osc1, oscNamespace.c_str(), "i", sensors.fsr);
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/touch/all");
            lo_send(osc1, oscNamespace.c_str(), "f", gestures.touchAll);
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/touch/top");
            lo_send(osc1, oscNamespace.c_str(), "f", gestures.touchTop);
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/touch/middle");
            lo_send(osc1, oscNamespace.c_str(), "f", gestures.touchMiddle);
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/touch/bottom");
            lo_send(osc1, oscNamespace.c_str(), "f", gestures.touchBottom);
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
    }
    if (puara.IP2_ready()) {
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "raw/capsense");
                lo_send(osc2, oscNamespace.c_str(), "iiiiiiiiiiiiiii", touch.touch[0], touch.touch[1],touch.touch[2],
                        touch.touch[3],touch.touch[4],touch.touch[5], touch.touch[6], touch.touch[7], touch.touch[8],
                        touch.touch[9], touch.touch[10], touch.touch[11], touch.touch[12], touch.touch[13], touch.touch[14]
                );
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "raw/fsr");
            lo_send(osc2, oscNamespace.c_str(), "i", sensors.fsr);
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/touch/all");
            lo_send(osc2, oscNamespace.c_str(), "f", gestures.touchAll);
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/touch/top");
            lo_send(osc2, oscNamespace.c_str(), "f", gestures.touchTop);
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/touch/middle");
            lo_send(osc2, oscNamespace.c_str(), "f", gestures.touchMiddle);
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/touch/bottom");
            lo_send(osc2, oscNamespace.c_str(), "f", gestures.touchBottom);
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
    }

    // Sending discrete OSC messages
    if (puara.IP1_ready()) {
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
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "battery");
            lo_send(osc1, oscNamespace.c_str(), "i", sensors.battery);
        }
    }
    if (puara.IP2_ready()) {
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
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "battery");
            lo_send(osc2, oscNamespace.c_str(), "i", sensors.battery);
        }
    }

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

    // run at 100 Hz
    //vTaskDelay(10 / portTICK_PERIOD_MS);
}
