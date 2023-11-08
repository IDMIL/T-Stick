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

// set the amount of capacitive stripes for 16, for IDMIl capsense board, or 15/30/60 for Trill Board
#define TSTICK_SIZE 30

/*
  Choose the capacitive sensing board
  - Trill
  - IDMIL Capsense board
*/
#define touch_TRILL
// #define touch_CAPSENSE

/*
 Define libmapper
*/
#define LIBMAPPER

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

#include <SparkFunLSM9DS1.h>
LSM9DS1 imu;

//////////////////////////////////////////////
// Include Touch stuff                      //
//////////////////////////////////////////////

#ifdef touch_TRILL
  #include "touch.h"
  Touch touch;
  Touch touch2;
  uint8_t touchI2C = 0x31;
  int mergedtouch[TSTICK_SIZE]; 
  int mergeddiscretetouch[TSTICK_SIZE]; 
  int mergednormalisedtouch[TSTICK_SIZE]; 
#endif

#ifdef touch_CAPSENSE
  #include "capsense.h"
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
} sensors;

struct Event {
    bool shake = false;
    bool jab = false;
    bool count = false;
    bool tap = false;
    bool dtap = false;
    bool ttap = false;
    bool brush = false;
    bool rub = false;
    bool battery;
} event;

void initIMU();

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
    initIMU();
    std::cout << "done" << std::endl;

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
        if (touch.initTouch()) {
            touch.touchSize = TSTICK_SIZE;
            std::cout << "done" << std::endl;
        } else {
            std::cout << "initialization failed!" << std::endl;
        }
        if (TSTICK_SIZE > 30) {
            if (touch2.initTouch(touchI2C)) {
                    touch.touchSize = 30;
                    touch2.touchSize = TSTICK_SIZE-30;
                    std::cout << "done" << std::endl;
                } else {
                    std::cout << "initialization failed!" << std::endl;
                }
        }
    #endif
    #ifdef touch_CAPSENSE
        capsense.capsense_scan(); // Look for Capsense boards and return their addresses
                                // must run before initLibmapper to get # of capsense boards
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
    lm.soc = mpr_sig_new(lm_dev, MPR_DIR_OUT, "battery", 1, MPR_FLT, "percent", &lm.batSOCMin, &lm.batSOCMax, 0, 0, 0);
    lm.batvolt = mpr_sig_new(lm_dev, MPR_DIR_OUT, "battery", 1, MPR_FLT, "percent", &lm.batVoltMin, &lm.batVoltMax, 0, 0, 0);
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

    //std::cout << gestures.getAccelX() << "," << gestures.getAccelY() << "," << gestures.getAccelZ() << "," <<
    //             gestures.getGyroX() << "," << gestures.getGyroY() << "," << gestures.getGyroZ() << "," <<
    //             gestures.getMagX() << "," << gestures.getMagY() << "," << gestures.getMagZ() << "\n";

    mpr_dev_poll(lm_dev, 0);

    button.readButton();
    
    fsr.readFsr();

    // Read Touch
    #ifdef touch_TRILL
        touch.readTouch();
        touch.cookData();
        if (TSTICK_SIZE>30) {
            touch2.readTouch();
            touch2.cookData();
        }
        for (int i = 0; i < TSTICK_SIZE; ++i) {
            if (i < 30) {
                mergedtouch[i] = touch.touch[i];
                mergeddiscretetouch[i] = touch.discreteTouch[i];
                mergednormalisedtouch[i] = touch.normTouch[i];
            } else {
                mergedtouch[i] = touch2.touch[i-30];
                mergeddiscretetouch[i] = touch2.discreteTouch[i-30];
                mergednormalisedtouch[i] = touch2.normTouch[i-30];
            }
        }
        gestures.updateTouchArray(mergeddiscretetouch,TSTICK_SIZE);
    #endif
    #ifdef touch_CAPSENSE
        capsense.readCapsense();
        gestures.updateTouchArray(capsense.data,capsense.touchStripsSize);
    #endif

    // read battery
    if (millis() - battery.interval > battery.timer) {
      battery.timer = millis();
      readBattery();
      batteryFilter();
    }

    // read IMU and update puara-gestures
        if (imu.accelAvailable()) {
        imu.readAccel();
        // In g's
        gestures.setAccelerometerValues(imu.calcAccel(imu.ax),
                                        imu.calcAccel(imu.ay),
                                        imu.calcAccel(imu.az));
    }
    if (imu.gyroAvailable()) {
        imu.readGyro();
        // In degrees/sec
        gestures.setGyroscopeValues(imu.calcGyro(imu.gx),
                                    imu.calcGyro(imu.gy),
                                    imu.calcGyro(imu.gz));
    }
    if (imu.magAvailable()) {
        imu.readMag();
        // In Gauss
        gestures.setMagnetometerValues(imu.calcMag(imu.mx),
                                       imu.calcMag(imu.my),
                                       imu.calcMag(imu.mz));
    }

    gestures.updateInertialGestures();
    gestures.updateTrigButton(button.getButton());

    // go to deep sleep if double press button
    if (gestures.getButtonDTap()){
        std::cout << "\nEntering deep sleep.\n\nGoodbye!\n" << std::endl;
        delay(1000);
        esp_deep_sleep_start();
    }

    // Preparing arrays for libmapper signals
    sensors.fsr = fsr.getValue();
    sensors.squeeze = fsr.getNormValue();
    // Convert accel from g's to meters/sec^2
    sensors.accl[0] = gestures.getAccelX() * 9.80665;
    sensors.accl[1] = gestures.getAccelY() * 9.80665;
    sensors.accl[2] = gestures.getAccelZ() * 9.80665;
    // Convert gyro from degrees/sec to radians/sec
    sensors.gyro[0] = gestures.getGyroX() * M_PI / 180;
    sensors.gyro[1] = gestures.getGyroY() * M_PI / 180;
    sensors.gyro[2] = gestures.getGyroZ() * M_PI / 180;
    // Convert mag from Gauss to uTesla
    sensors.magn[0] = gestures.getMagX() / 10000;
    sensors.magn[1] = gestures.getMagY() / 10000;
    sensors.magn[2] = gestures.getMagZ() / 10000;
    // Orientation quaternion
    sensors.quat[0] = gestures.getOrientationQuaternion().w;
    sensors.quat[1] = gestures.getOrientationQuaternion().x;
    sensors.quat[2] = gestures.getOrientationQuaternion().y;
    sensors.quat[3] = gestures.getOrientationQuaternion().z;
    // Yaw (heading), pitch (tilt) and roll
    sensors.ypr[0] = gestures.getYaw();
    sensors.ypr[1] = gestures.getPitch();
    sensors.ypr[2] = gestures.getRoll();
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
    #ifdef touch_TRILL
        mpr_sig_set_value(lm.rawtouch, 0, TSTICK_SIZE, MPR_INT32, &mergedtouch);
        mpr_sig_set_value(lm.disctouch, 0, TSTICK_SIZE, MPR_INT32, &mergeddiscretetouch);
    #endif
    #ifdef touch_CAPSENSE
        mpr_sig_set_value(lm.rawtouch, 0, capsense.touchStripsSize, MPR_INT32, &capsense.data);
    #endif

    // Sending continuous OSC messages
    // Sending continuous OSC messages
    if (puara.IP1_ready()) {
            #ifdef touch_TRILL
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
                } else if (TSTICK_SIZE == 45) {
                    // Send data from the first board
                    lo_send(osc1, oscNamespace.c_str(), "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii", mergedtouch[0], mergedtouch[1],mergedtouch[2],
                    mergedtouch[3],mergedtouch[4],mergedtouch[5], mergedtouch[6], mergedtouch[7], mergedtouch[8],
                    mergedtouch[9], mergedtouch[10], mergedtouch[11], mergedtouch[12], mergedtouch[13], mergedtouch[14], mergedtouch[15], mergedtouch[16],mergedtouch[17],
                    mergedtouch[18],mergedtouch[19],mergedtouch[20], mergedtouch[21], mergedtouch[22], mergedtouch[23],
                    mergedtouch[24], mergedtouch[25], mergedtouch[26], mergedtouch[27], mergedtouch[28], mergedtouch[29], mergedtouch[30], mergedtouch[31], 
                    mergedtouch[32], mergedtouch[33], mergedtouch[34], mergedtouch[35], mergedtouch[36], mergedtouch[37], mergedtouch[38], mergedtouch[39], 
                    mergedtouch[40], mergedtouch[41], mergedtouch[42], mergedtouch[43], mergedtouch[44]);
                    oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/touch/normalised");
                    lo_send(osc1, oscNamespace.c_str(), "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii", mergednormalisedtouch[0], mergednormalisedtouch[1],mergednormalisedtouch[2],
                    mergednormalisedtouch[3],mergednormalisedtouch[4],mergednormalisedtouch[5], mergednormalisedtouch[6], mergednormalisedtouch[7], mergednormalisedtouch[8],
                    mergednormalisedtouch[9], mergednormalisedtouch[10], mergednormalisedtouch[11], mergednormalisedtouch[12], mergednormalisedtouch[13], mergednormalisedtouch[14], mergednormalisedtouch[15], mergednormalisedtouch[16],mergednormalisedtouch[17],
                    mergednormalisedtouch[18],mergednormalisedtouch[19],mergednormalisedtouch[20], mergednormalisedtouch[21], mergednormalisedtouch[22], mergednormalisedtouch[23],
                    mergednormalisedtouch[24], mergednormalisedtouch[25], mergednormalisedtouch[26], mergednormalisedtouch[27], mergednormalisedtouch[28], mergednormalisedtouch[29], mergednormalisedtouch[30], mergednormalisedtouch[31], 
                    mergednormalisedtouch[32], mergednormalisedtouch[33], mergednormalisedtouch[34], mergednormalisedtouch[35], mergednormalisedtouch[36], mergednormalisedtouch[37], mergednormalisedtouch[38], mergednormalisedtouch[39], 
                    mergednormalisedtouch[40], mergednormalisedtouch[41], mergednormalisedtouch[42], mergednormalisedtouch[43], mergednormalisedtouch[44]);
                    oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/touch/discrete");
                    lo_send(osc1, oscNamespace.c_str(), "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii", mergeddiscretetouch[0], mergeddiscretetouch[1],mergeddiscretetouch[2],
                    mergeddiscretetouch[3],mergeddiscretetouch[4],mergeddiscretetouch[5], mergeddiscretetouch[6], mergeddiscretetouch[7], mergeddiscretetouch[8],
                    mergeddiscretetouch[9], mergeddiscretetouch[10], mergeddiscretetouch[11], mergeddiscretetouch[12], mergeddiscretetouch[13], mergeddiscretetouch[14], mergeddiscretetouch[15], mergeddiscretetouch[16],mergeddiscretetouch[17],
                    mergeddiscretetouch[18],mergeddiscretetouch[19],mergeddiscretetouch[20], mergeddiscretetouch[21], mergeddiscretetouch[22], mergeddiscretetouch[23],
                    mergeddiscretetouch[24], mergeddiscretetouch[25], mergeddiscretetouch[26], mergeddiscretetouch[27], mergeddiscretetouch[28], mergeddiscretetouch[29], mergeddiscretetouch[30], mergeddiscretetouch[31], 
                    mergeddiscretetouch[32], mergeddiscretetouch[33], mergeddiscretetouch[34], mergeddiscretetouch[35], mergeddiscretetouch[36], mergeddiscretetouch[37], mergeddiscretetouch[38], mergeddiscretetouch[39], 
                    mergeddiscretetouch[40], mergeddiscretetouch[41], mergeddiscretetouch[42], mergeddiscretetouch[43], mergeddiscretetouch[44]);
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
                }
                else {
                    lo_send(osc1, oscNamespace.c_str(), "iiiiiiiiiiiiiii", mergedtouch[0], mergedtouch[1], mergedtouch[2],
                        mergedtouch[3],mergedtouch[4],mergedtouch[5], mergedtouch[6], mergedtouch[7], mergedtouch[8],
                        mergedtouch[9], mergedtouch[10], mergedtouch[11], mergedtouch[12], mergedtouch[13], mergedtouch[14]);
                    oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/touch/normalised");
                    lo_send(osc1, oscNamespace.c_str(), "iiiiiiiiiiiiiii", mergednormalisedtouch[0], mergednormalisedtouch[1], mergednormalisedtouch[2],
                        mergednormalisedtouch[3],mergednormalisedtouch[4],mergednormalisedtouch[5], mergednormalisedtouch[6], mergednormalisedtouch[7], mergednormalisedtouch[8],
                        mergednormalisedtouch[9], mergednormalisedtouch[10], mergednormalisedtouch[11], mergednormalisedtouch[12], mergednormalisedtouch[13], mergednormalisedtouch[14]);                   
                    oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/touch/discrete");
                    lo_send(osc1, oscNamespace.c_str(), "iiiiiiiiiiiiiii", mergeddiscretetouch[0], mergeddiscretetouch[1], mergeddiscretetouch[2],
                        mergeddiscretetouch[3],mergeddiscretetouch[4],mergeddiscretetouch[5], mergeddiscretetouch[6], mergeddiscretetouch[7], mergeddiscretetouch[8],
                        mergeddiscretetouch[9], mergeddiscretetouch[10], mergeddiscretetouch[11], mergeddiscretetouch[12], mergeddiscretetouch[13], mergeddiscretetouch[14]);
                }
            #endif
            #ifdef touch_CAPSENSE
                lo_send(osc1, oscNamespace.c_str(), "iiiiiiiiiiiiiiii", capsense.data[0], capsense.data[1],capsense.data[2],
                    capsense.data[3],capsense.data[4],capsense.data[5], capsense.data[6], capsense.data[7], capsense.data[8],
                    capsense.data[9], capsense.data[10], capsense.data[11], capsense.data[12], capsense.data[13], capsense.data[14],
                    capsense.data[15]
            );
            #endif
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "raw/fsr");
            lo_send(osc1, oscNamespace.c_str(), "i", sensors.fsr);
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/squeeze");
            lo_send(osc1, oscNamespace.c_str(), "f", sensors.squeeze);
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
            #ifdef touch_TRILL
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
                } else if (TSTICK_SIZE == 45) {
                    // Send data from the first board
                    lo_send(osc2, oscNamespace.c_str(), "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii", mergedtouch[0], mergedtouch[1],mergedtouch[2],
                    mergedtouch[3],mergedtouch[4],mergedtouch[5], mergedtouch[6], mergedtouch[7], mergedtouch[8],
                    mergedtouch[9], mergedtouch[10], mergedtouch[11], mergedtouch[12], mergedtouch[13], mergedtouch[14], mergedtouch[15], mergedtouch[16],mergedtouch[17],
                    mergedtouch[18],mergedtouch[19],mergedtouch[20], mergedtouch[21], mergedtouch[22], mergedtouch[23],
                    mergedtouch[24], mergedtouch[25], mergedtouch[26], mergedtouch[27], mergedtouch[28], mergedtouch[29], mergedtouch[30], mergedtouch[31], 
                    mergedtouch[32], mergedtouch[33], mergedtouch[34], mergedtouch[35], mergedtouch[36], mergedtouch[37], mergedtouch[38], mergedtouch[39], 
                    mergedtouch[40], mergedtouch[41], mergedtouch[42], mergedtouch[43], mergedtouch[44]);
                    oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/touch/normalised");
                    lo_send(osc2, oscNamespace.c_str(), "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii", mergednormalisedtouch[0], mergednormalisedtouch[1],mergednormalisedtouch[2],
                    mergednormalisedtouch[3],mergednormalisedtouch[4],mergednormalisedtouch[5], mergednormalisedtouch[6], mergednormalisedtouch[7], mergednormalisedtouch[8],
                    mergednormalisedtouch[9], mergednormalisedtouch[10], mergednormalisedtouch[11], mergednormalisedtouch[12], mergednormalisedtouch[13], mergednormalisedtouch[14], mergednormalisedtouch[15], mergednormalisedtouch[16],mergednormalisedtouch[17],
                    mergednormalisedtouch[18],mergednormalisedtouch[19],mergednormalisedtouch[20], mergednormalisedtouch[21], mergednormalisedtouch[22], mergednormalisedtouch[23],
                    mergednormalisedtouch[24], mergednormalisedtouch[25], mergednormalisedtouch[26], mergednormalisedtouch[27], mergednormalisedtouch[28], mergednormalisedtouch[29], mergednormalisedtouch[30], mergednormalisedtouch[31], 
                    mergednormalisedtouch[32], mergednormalisedtouch[33], mergednormalisedtouch[34], mergednormalisedtouch[35], mergednormalisedtouch[36], mergednormalisedtouch[37], mergednormalisedtouch[38], mergednormalisedtouch[39], 
                    mergednormalisedtouch[40], mergednormalisedtouch[41], mergednormalisedtouch[42], mergednormalisedtouch[43], mergednormalisedtouch[44]);
                    oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/touch/discrete");
                    lo_send(osc2, oscNamespace.c_str(), "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii", mergeddiscretetouch[0], mergeddiscretetouch[1],mergeddiscretetouch[2],
                    mergeddiscretetouch[3],mergeddiscretetouch[4],mergeddiscretetouch[5], mergeddiscretetouch[6], mergeddiscretetouch[7], mergeddiscretetouch[8],
                    mergeddiscretetouch[9], mergeddiscretetouch[10], mergeddiscretetouch[11], mergeddiscretetouch[12], mergeddiscretetouch[13], mergeddiscretetouch[14], mergeddiscretetouch[15], mergeddiscretetouch[16],mergeddiscretetouch[17],
                    mergeddiscretetouch[18],mergeddiscretetouch[19],mergeddiscretetouch[20], mergeddiscretetouch[21], mergeddiscretetouch[22], mergeddiscretetouch[23],
                    mergeddiscretetouch[24], mergeddiscretetouch[25], mergeddiscretetouch[26], mergeddiscretetouch[27], mergeddiscretetouch[28], mergeddiscretetouch[29], mergeddiscretetouch[30], mergeddiscretetouch[31], 
                    mergeddiscretetouch[32], mergeddiscretetouch[33], mergeddiscretetouch[34], mergeddiscretetouch[35], mergeddiscretetouch[36], mergeddiscretetouch[37], mergeddiscretetouch[38], mergeddiscretetouch[39], 
                    mergeddiscretetouch[40], mergeddiscretetouch[41], mergeddiscretetouch[42], mergeddiscretetouch[43], mergeddiscretetouch[44]);
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
                }
                else {
                    lo_send(osc2, oscNamespace.c_str(), "iiiiiiiiiiiiiii", mergedtouch[0], mergedtouch[1], mergedtouch[2],
                        mergedtouch[3],mergedtouch[4],mergedtouch[5], mergedtouch[6], mergedtouch[7], mergedtouch[8],
                        mergedtouch[9], mergedtouch[10], mergedtouch[11], mergedtouch[12], mergedtouch[13], mergedtouch[14]);
                    oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/touch/normalised");
                    lo_send(osc2, oscNamespace.c_str(), "iiiiiiiiiiiiiii", mergednormalisedtouch[0], mergednormalisedtouch[1], mergednormalisedtouch[2],
                        mergednormalisedtouch[3],mergednormalisedtouch[4],mergednormalisedtouch[5], mergednormalisedtouch[6], mergednormalisedtouch[7], mergednormalisedtouch[8],
                        mergednormalisedtouch[9], mergednormalisedtouch[10], mergednormalisedtouch[11], mergednormalisedtouch[12], mergednormalisedtouch[13], mergednormalisedtouch[14]);                   
                    oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/touch/discrete");
                    lo_send(osc2, oscNamespace.c_str(), "iiiiiiiiiiiiiii", mergeddiscretetouch[0], mergeddiscretetouch[1], mergeddiscretetouch[2],
                        mergeddiscretetouch[3],mergeddiscretetouch[4],mergeddiscretetouch[5], mergeddiscretetouch[6], mergeddiscretetouch[7], mergeddiscretetouch[8],
                        mergeddiscretetouch[9], mergeddiscretetouch[10], mergeddiscretetouch[11], mergeddiscretetouch[12], mergeddiscretetouch[13], mergeddiscretetouch[14]);
                }
            #endif
            #ifdef touch_CAPSENSE
                oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "raw/capsense");
                lo_send(osc2, oscNamespace.c_str(), "iiiiiiiiiiiiiiii", capsense.data[0], capsense.data[1],capsense.data[2],
                    capsense.data[3],capsense.data[4],capsense.data[5], capsense.data[6], capsense.data[7], capsense.data[8],
                    capsense.data[9], capsense.data[10], capsense.data[11], capsense.data[12], capsense.data[13], capsense.data[14],
                    capsense.data[15]
            );
            #endif
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "raw/fsr");
            lo_send(osc2, oscNamespace.c_str(), "i", sensors.fsr);
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "instrument/squeeze");
            lo_send(osc2, oscNamespace.c_str(), "f", sensors.squeeze);
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
            // Battery Data
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "battery/percentage");
            lo_send(osc1, oscNamespace.c_str(), "i", sensors.battery);       
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "battery/voltage");
            lo_send(osc1, oscNamespace.c_str(), "f", battery.value);       
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
            // Battery Data
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "battery/percentage");
            lo_send(osc2, oscNamespace.c_str(), "i", sensors.battery);       
            oscNamespace.replace(oscNamespace.begin()+baseNamespace.size(),oscNamespace.end(), "battery/voltage");
            lo_send(osc2, oscNamespace.c_str(), "f", battery.value);       
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
}

void initIMU() {
    Wire.begin();

    // [enabled] turns the gyro on or off.
    imu.settings.gyro.enabled = true;
    // [scale] sets the full-scale range of the gyroscope.
    // scale can be set to either 245, 500, or 2000 dps
    // Travis West 2022-11-02: I was able to saturate the output with 2000 dps, so this seems like an appropriate setting.
    imu.settings.gyro.scale = 2000;
    // [sampleRate] sets the output data rate (ODR) of the gyro
    // sampleRate can be set between 1-6
    // 1 = 14.9    4 = 238
    // 2 = 59.5    5 = 476
    // 3 = 119     6 = 952
    imu.settings.gyro.sampleRate = 3; // 59.5Hz ODR
    // [bandwidth] can set the cutoff frequency of the gyro.
    // Allowed values: 0-3. Actual value of cutoff frequency
    // depends on the sample rate. (Datasheet section 7.12)
    imu.settings.gyro.bandwidth = 0;
    // [lowPowerEnable] turns low-power mode on or off.
    imu.settings.gyro.lowPowerEnable = false; // LP mode off
    // [HPFEnable] enables or disables the high-pass filter
    imu.settings.gyro.HPFEnable = true; // HPF disabled
    // [HPFCutoff] sets the HPF cutoff frequency (if enabled)
    // Allowable values are 0-9. Value depends on ODR.
    // (Datasheet section 7.14)
    imu.settings.gyro.HPFCutoff = 1; // HPF cutoff = 4Hz
    // [flipX], [flipY], and [flipZ] are booleans that can
    // automatically switch the positive/negative orientation
    // of the three gyro axes.
    imu.settings.gyro.flipX = false; // Don't flip X
    imu.settings.gyro.flipY = false; // Don't flip Y
    imu.settings.gyro.flipZ = false; // Don't flip Z
    // [enabled] turns the acclerometer on or off.
    imu.settings.accel.enabled = true; // Enable accelerometer
    // [enableX], [enableY], and [enableZ] can turn on or off
    // select axes of the acclerometer.
    imu.settings.accel.enableX = true; // Enable X
    imu.settings.accel.enableY = true; // Enable Y
    imu.settings.accel.enableZ = true; // Enable Z
    // [scale] sets the full-scale range of the accelerometer.
    // accel scale can be 2, 4, 8, or 16 g's
    // Travis West 2022-11-02: In my experiments I found that the effort required
    // to get much more than 7.5 g of acceleration was significant enough that I
    // was worried about causing damage the internal wiring of the instrument.
    // As such, I think 8 g full scale range or less is appropriate, at least
    // until such time as the mechanical robustness of the instrument is improved.
    imu.settings.accel.scale = 8;
    // [sampleRate] sets the output data rate (ODR) of the
    // accelerometer. ONLY APPLICABLE WHEN THE GYROSCOPE IS
    // DISABLED! Otherwise accel sample rate = gyro sample rate.
    // accel sample rate can be 1-6
    // 1 = 10 Hz    4 = 238 Hz
    // 2 = 50 Hz    5 = 476 Hz
    // 3 = 119 Hz   6 = 952 Hz
    imu.settings.accel.sampleRate = 3;
    // [bandwidth] sets the anti-aliasing filter bandwidth.
    // Accel cutoff frequency can be any value between -1 - 3. 
    // -1 = bandwidth determined by sample rate
    // 0 = 408 Hz   2 = 105 Hz
    // 1 = 211 Hz   3 = 50 Hz
    imu.settings.accel.bandwidth = 0; // BW = 408Hz
    // [highResEnable] enables or disables high resolution 
    // mode for the acclerometer.
    imu.settings.accel.highResEnable = false; // Disable HR
    // [highResBandwidth] sets the LP cutoff frequency of
    // the accelerometer if it's in high-res mode.
    // can be any value between 0-3
    // LP cutoff is set to a factor of sample rate
    // 0 = ODR/50    2 = ODR/9
    // 1 = ODR/100   3 = ODR/400
    imu.settings.accel.highResBandwidth = 0;  
    // [enabled] turns the magnetometer on or off.
    imu.settings.mag.enabled = true; // Enable magnetometer
    // [scale] sets the full-scale range of the magnetometer
    // mag scale can be 4, 8, 12, or 16 Gs
    // Travis West 2022-11-02: Considering that the Earth's magnetic field is
    // generally less than 1 Gs, the lowest setting available is likely best.
    // A higher setting could be used if the sensor were installed next to a
    // strong magnetic field, such as a magnet or speaker, since then the reading
    // would not be saturated and the bias from the magnet could potentially be
    // removed.
    imu.settings.mag.scale = 4;
    // [sampleRate] sets the output data rate (ODR) of the
    // magnetometer.
    // mag data rate can be 0-7:
    // 0 = 0.625 Hz  4 = 10 Hz
    // 1 = 1.25 Hz   5 = 20 Hz
    // 2 = 2.5 Hz    6 = 40 Hz
    // 3 = 5 Hz      7 = 80 Hz
    imu.settings.mag.sampleRate = 5; // Set OD rate to 20Hz
    // [tempCompensationEnable] enables or disables 
    // temperature compensation of the magnetometer.
    imu.settings.mag.tempCompensationEnable = false;
    // [XYPerformance] sets the x and y-axis performance of the
    // magnetometer to either:
    // 0 = Low power mode      2 = high performance
    // 1 = medium performance  3 = ultra-high performance
    imu.settings.mag.XYPerformance = 3; // Ultra-high perform.
    // [ZPerformance] does the same thing, but only for the z
    imu.settings.mag.ZPerformance = 3; // Ultra-high perform.
    // [lowPowerEnable] enables or disables low power mode in
    // the magnetometer.
    imu.settings.mag.lowPowerEnable = false;
    // [operatingMode] sets the operating mode of the
    // magnetometer. operatingMode can be 0-2:
    // 0 = continuous conversion
    // 1 = single-conversion
    // 2 = power down
    imu.settings.mag.operatingMode = 0; // Continuous mode

    imu.begin();
}