// T-Stick firmware - Main file
// Edu Meneses
// IDMIL - McGill University (2022)

/*
OBS: if you face a fatal error related to the file  ../../../lwip/src/include/lwip/inet.h,
     edit the inet.h file from 
        #include "../../../lwip/src/include/lwip/inet.h"
     to
        #include "lwip/inet.h"

 For more info: https://github.com/mathiasbredholt/libmapper-arduino/issues/3
*/

const unsigned int firmware_version = 220102;

// The T-Stick usually has the  uses the LSM9DS1
  //#define imu_BNO080
  #define imu_LSM9DS1

// If using TinyPICO (current HW) version or Wemos D32 Pro
  #define board_TINYPICO
  //#define board_WEMOS

// Turn everything related to libmapper off
//#define DISABLE_LIBMAPPER

// Turn everything related to MIDI off
#define DISABLE_MIDI

#include <Arduino.h>
#include <Update.h>       // For OTA over web server (manual .bin upload)

#ifndef DISABLE_LIBMAPPER
  //#include <string>
  #include <mapper_cpp.h> // libmapper
  //using namespace mapper;
#endif

#include <deque>
#include <cmath>          // std::abs
#include <algorithm>      // std::min_element, std::max_element

#include "mdns.h"

#include <OSCMessage.h>   // https://github.com/CNMAT/OSC
#include <OSCBundle.h>

/////////////////////
// Pin definitions //
/////////////////////

  struct Pin {
      int led;    // Built In LED pin
      int touch;  // Capacitive touch pin
      int battery;// To check battery level (voltage)
      int ultTrig;// connects to the trigger pin on the distance sensor
      int ultEcho;// connects to the echo pin on the distance sensor
    };

  #ifdef board_WEMOS
    Pin pin{ 5, 15, 35, 32, 33 };
  #elif defined(board_TINYPICO)
    Pin pin{ 5, 4, 35, 32, 33 };
    #include <TinyPICO.h>
    TinyPICO tinypico = TinyPICO();
  #endif

///////////////////////////////
// Firmware global variables //
///////////////////////////////

  struct global {
    char deviceName[25];
    char APpasswd[15];
    bool rebootFlag = false;
    bool midiFlag = false;
    unsigned long rebootTimer;
    unsigned int oscDelay = 10; // 10ms ~= 100Hz
    unsigned int midiDelay = 20; // 20ms ~= 50Hz
    unsigned long messageTimer = 0;
    unsigned int lastCount = 1;
    unsigned int lastTap = 1;
    unsigned int lastDtap = 1;
    unsigned int lastTtap = 1;
    unsigned int lastTrig = 1;
    unsigned int lastDistance = 1;
    float lastShake[3] = {0.1, 0.1, 0.1};
    float lastJab[3] = {0.1, 0.1, 0.1};
    int ledValue = 0;
    uint8_t color = 0;
  } global;


//////////////
// settings //
//////////////

  struct Settings {
    int id = 6;
    char author[20] = {'E','d','u','_','M','e','n','e','s','e','s'};
    char institution[20] = {'I','D','M','I','L'};
    char APpasswd[15] = {'m','a','p','p','i','n','g','s'};
    char lastConnectedNetwork[30] = {'I','D','M','I','L'};
    char lastStoredPsk[30] = {'m','a','p','p','i','n','g','s'};
    int32_t firmware = 0;
    char oscIP[2][17] = { {'0','0','0','0'}, {'0','0','0','0'} };
    int32_t oscPORT[2] = {8000, 8000};
    int32_t localPORT = 8000;
    bool libmapper = false;
    bool osc = false;
    int mode = 0; // 0:STA, 1:AP, 2:MIDI, 3:Setup(STA+AP+WebServer)
    int oldMode = 0; // saves stored mode before enter setup
    int touchSensitivity = 0;
  } settings;
  

////////////////////////////////////////////////////////////
// Include T-Stick WiFi and json module functions files //
////////////////////////////////////////////////////////////

  #include "module.h"

  DNSServer dnsServer;
  AsyncWebServer server(80);
  Module module;
  WiFiUDP oscEndpoint; // A UDP instance to let us send and receive packets over UDP  

  void sendOSC(char* ip,int32_t port, const char* messageNamespace, float data);
  void sendTrioOSC(char* ip,int32_t port, const char* messageNamespace, float data1, float data2, float data3);
  void sendContinuousOSC(char* ip, int32_t port);
  void sendInfo(OSCMessage &msg);
  void receiveOSC();
  
////////////////////////////////////////////////////////
// Libmapper stuff with specific forward declarations //
////////////////////////////////////////////////////////

#ifndef DISABLE_LIBMAPPER
  mapper::Device* dev;
  //char nome[] = "teste";
  //mapper::Device dev("test");

  struct lm {
    mapper::Signal ult;
    int ultMax = 500;
    int ultMin = 0;
    mapper::Signal accel;
    float accelMax = 50;
    float accelMin = -50;
    mapper::Signal gyro;
    float gyroMax = 25;
    float gyroMin = -25;
    mapper::Signal mag;
    float magMax = 25;
    float magMin = -25;
    mapper::Signal quat;
    float quatMax = 1;
    float quatMin = -1;
    mapper::Signal euler;
    float eulerMax = 180;
    float eulerMin = -180;
    mapper::Signal shake;
    float shakeMax =  50;
    float shakeMin = -50;
    mapper::Signal jab;
    float jabMax = 50;
    float jabMin = -50;
    mapper::Signal count;
    int countMax = 100;
    int countMin = 0;
    mapper::Signal tap;
    mapper::Signal ttap;
    mapper::Signal dtap;
    int tapMax = 1;
    int tapMin = 0;
    mapper::Signal bat;
    int batMax = 100;
    int batMin = 0;
  } lm;

  void initLibmapper();
  void updateLibmapper();

#endif

////////////////////////////////////////
// Include Ultrassonic function files //
////////////////////////////////////////

  #include "ult.h"
  
  struct UltData {
    unsigned int distance;
    unsigned int tempDistance;
    unsigned int lastDistance;
    unsigned int ultMaxDistance = 20; // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.
    unsigned int ultMinDistance = 20; // Maximum distance considered (IN MILLIMETERS)
    int trigger;
    unsigned long timer = 0;
    unsigned long trigTimer = 0;
    int interval = 20; // in ms (1/f)
    int trigInterval = 200; // in ms (1/f)
    int queueAmount = 10; // # of values stored
    std::deque<int> filterArray; // store last values
  } ultData;

  NewPing ult(pin.ultTrig, pin.ultEcho, ultData.ultMaxDistance); // NewPing setup of pins and maximum distance.

  void ultFilter();
  void readUlt();
  void readUltTrigger();

///////////////////
// Battery stuff //
///////////////////
  
  struct BatteryData {
    unsigned int percentage = 0;
    unsigned int lastPercentage = 0;
    float value;
    unsigned long timer = 0;
    int interval = 1000; // in ms (1/f)
    int queueAmount = 10; // # of values stored
    std::deque<int> filterArray; // store last values
  } battery;

  void readBattery();
  void batteryFilter();

//////////////////////////////////
// Include Touch function files //
//////////////////////////////////

  #include "touch.h"

  Touch touch;

////////////////////////////////
// Include IMU function files //
////////////////////////////////

  //Choose the file to include according to the IMU used
  
  #ifdef imu_BNO080
     #include "bno080.h" 
        Imu_BNO080 imu;
  #endif
  #ifdef imu_LSM9DS1
    #include "lsm9ds1.h"
        Imu_LSM9DS1 imu;
  #endif

//////////////////////////////////////////
// Include MIDI libraries and functions //
//////////////////////////////////////////

  #ifndef DISABLE_MIDI

  #include "midi.h"

  Midi midi;

  struct MidiReady {
    unsigned int AccelX; // CC 021 (0b00010101) (for X-axis accelerometer)
    unsigned int AccelZ; // CC 023 (0b00010111) (for Z-axis accelerometer)
    unsigned int AccelY; // CC 022 (0b00010110) (for Y-axis accelerometer)
    unsigned int GyroX;  // CC 024 (0b00011000) (for X-axis gyroscope)
    unsigned int GyroY;  // CC 025 (0b00011001) (for Y-axis gyroscope)
    unsigned int GyroZ;  // CC 026 (0b00011010) (for Z-axis gyroscope)
    unsigned int Yaw;    // CC 027 (0b00011011) (for yaw)
    unsigned int Pitch;  // CC 028 (0b00011100) (for pitch)
    unsigned int Roll;   // CC 029 (0b00011101) (for roll)
    unsigned int ShakeX;
    unsigned int ShakeY;
    unsigned int ShakeZ;
    unsigned int JabX;
    unsigned int JabY;
    unsigned int JabZ;
  } midiReady;

  #endif

/////////////////////////////////////////
// Include LED libraries and functions //
/////////////////////////////////////////

  #include "led.h"

  Led led;

  float hardBlink(unsigned long &timer, int interval, float currentValue);
  float rampBlink(unsigned long &timer, int interval, float currentValue, bool &direction);

//////////////////////////////////////////////
// Include High-level descriptors functions //
//////////////////////////////////////////////

  #include "instrument.h"

  Instrument instrument;

//////////////////////////
// Forward declarations //
//////////////////////////

  void printVariables();
  void parseJSON();
  void saveJSON();
  void initWebServer();
  void start_mdns_service();
  String indexProcessor(const String& var);
  String scanProcessor(const String& var);
  String factoryProcessor(const String& var);
  String updateProcessor(const String& var);


/////////////
/////////////
/// SETUP ///
/////////////
/////////////

void setup() {
  // put your setup code here, to run once:
    Serial.begin(115200);

  #ifdef board_WEMOS
    // LED init for WEMOS
      ledcSetup(0, 5000, 8);
      ledcAttachPin(pin.led, 0);
  #endif

  // Start FS and check Json file (config.json)
    module.mountFS();

  // Load Json file stored values
    parseJSON();
    settings.firmware = firmware_version;
    if (settings.mode < 0 || settings.mode > 3 ){
      settings.mode = 3;
      Serial.println("Mode error: changing to setup mode for correction");
      saveJSON();
    }

  printf( "\n"
        "T-Stick\n"
        "Edu Meneses - IDMIL - CIRMMT - McGill University\n"
        "module ID: %03i\n"
        "Version %d\n"
        "\n"
        "Booting System...\n",settings.id,settings.firmware);

  // Print Json file stored values
    //printJSON();
    printVariables();

  // Define this module full name
    snprintf(global.deviceName,(sizeof(global.deviceName)-1),"T-Stick_module_%03i",settings.id);

  // Connect to WiFi and start dns server
    #ifndef DISABLE_MIDI

    if (settings.mode != 2) {
      module.scanWiFi(global.deviceName, settings.mode, settings.APpasswd, settings.lastConnectedNetwork, settings.lastStoredPsk);
      global.midiFlag = false;
    }

    #else

    module.scanWiFi(global.deviceName, settings.mode, settings.APpasswd, settings.lastConnectedNetwork, settings.lastStoredPsk);
    global.midiFlag = false;

    #endif
    
    module.startWifi(global.deviceName, settings.mode, settings.APpasswd, settings.lastConnectedNetwork, settings.lastStoredPsk);

  // Start listening for incoming OSC messages if WiFi is ON
    if (settings.mode != 2) {
      oscEndpoint.begin(settings.localPORT);
      Serial.println("Starting UDP (listening to OSC messages)");
      Serial.print("Local port: "); Serial.println(settings.localPORT);
      global.midiFlag = false;
    }

  // Initializing touch (set pin)
    Serial.print("    Initializing Capacitive touch sensor...  ");
    if (touch.initTouch()) {
      touch.setSensitivity(settings.touchSensitivity);
      Serial.println("done");
    } else {
      Serial.println("Capacitive touch sensor initialization failed!");
    }

  // Initializing IMU
    Serial.print("    Initializing IMU...  ");
    if (imu.initIMU()) {
      Serial.println("done");
    } else {
      Serial.println("IMU initialization failed!");
    }

  // No need to initialize the Ultrasonic sensor

  // Initializing MIDI if in MIDI mode (settings.mode = 2)

    #ifndef DISABLE_MIDI

    if (settings.mode == 2) {
      Serial.print("    Initializing BLE MIDI...  ");
      midi.setDeviceName(global.deviceName);
      if (midi.initMIDI()) {
        Serial.print("MIDI mode: Connect to "); Serial.print(global.deviceName); Serial.println(" to start sending BLE MIDI");
        Serial.print("    (channel "); Serial.print(midi.getChannel()); Serial.println(")");
      } else {
        Serial.println("BLE MIDI initialization failed!");
      }
      global.midiFlag = true;
    }

    #endif

  #ifndef DISABLE_LIBMAPPER
  // Init libmapper if option is ON
    if (settings.libmapper && !global.midiFlag) {
      Serial.print("    Starting libmapper... ");
      //mapper::Device dev("name");
      initLibmapper();
      Serial.println("done");
    }
  #endif

  // Start dns and web Server if in setup mode
  if (settings.mode == 3) {
        Serial.println("Starting DNS server");
        dnsServer.start(53, "*", WiFi.softAPIP());
        start_mdns_service();
        initWebServer();
        global.midiFlag = false;
      }
    
  Serial.println("\n\nBoot complete\n\n");

} // end Setup


////////////
////////////
/// LOOP ///
////////////
////////////

void loop() {

  // Check for serial messages. This firmware accepts:
  //     - 's' to start setup mode
  //     - 'r' to reboot
  //     - 'd' to enter deep sleep
  if (Serial.available() > 0) {
    int incomingByte = 0;
    incomingByte = Serial.read();

    // say what you got:
    Serial.print("I received: ");
    Serial.println(incomingByte, DEC);
  }

  if (settings.mode == 3) {
    dnsServer.processNextRequest();
  }

  // Read Ultrasonic sensor distance
    //readUlt();
    readUltTrigger();

  // Read capacitive button
    touch.readTouch();

  // read battery
    if (millis() - battery.interval > battery.timer) {
      battery.timer = millis();
      readBattery();
      batteryFilter();
    }

  // Get High-level descriptors (instrument data) - jab and shake for now
    instrument.updateInstrument(imu.getGyroX(), imu.getGyroY(), imu.getGyroZ());

  // prepare MIDI data if needed

    #ifndef DISABLE_MIDI

    if (global.midiFlag) {
      midiReady.AccelX = midi.mapMIDI(abs(imu.getAccelX()), 0, 50);
      midiReady.AccelZ = midi.mapMIDI(abs(imu.getAccelY()), 0, 50);
      midiReady.AccelY = midi.mapMIDI(abs(imu.getAccelZ()), 0, 50);
      midiReady.GyroX = midi.mapMIDI(abs(imu.getGyroX()), 0, 25);
      midiReady.GyroY = midi.mapMIDI(abs(imu.getGyroY()), 0, 25);
      midiReady.GyroZ = midi.mapMIDI(abs(imu.getGyroZ()), 0, 25);
      midiReady.Yaw  = midi.mapMIDI(imu.getYaw(), -180, 180);
      midiReady.Pitch = midi.mapMIDI(imu.getPitch(), -180, 180);
      midiReady.Roll  = midi.mapMIDI(imu.getRoll(), -180, 180);
      midiReady.ShakeX = midi.mapMIDI(instrument.getShakeX(), 0, 50);
      midiReady.ShakeY = midi.mapMIDI(instrument.getShakeY(), 0, 50);
      midiReady.ShakeZ = midi.mapMIDI(instrument.getShakeZ(), 0, 50);
      midiReady.JabX = midi.mapMIDI(instrument.getJabX(), 0, 50);
      midiReady.JabY = midi.mapMIDI(instrument.getJabY(), 0, 50);
      midiReady.JabZ = midi.mapMIDI(instrument.getJabZ(), 0, 50);
    }

    #endif

  // send data via OSC or MIDI
    if (!global.midiFlag && settings.osc) {
      if (settings.mode==1 || WiFi.status() == WL_CONNECTED) { // Send data via OSC ...
          // sending continuous data
            if (millis() - global.oscDelay > global.messageTimer) { 
              global.messageTimer = millis();
              sendContinuousOSC(settings.oscIP[0], settings.oscPORT[0]);
              sendContinuousOSC(settings.oscIP[1], settings.oscPORT[1]);
            }
          // send discrete (touch/battery) data (only when it changes) or != 0
            if (global.lastCount != touch.getCount()) {
              sendOSC(settings.oscIP[0], settings.oscPORT[0], "count", touch.getCount());
              sendOSC(settings.oscIP[1], settings.oscPORT[1], "count", touch.getCount());
              global.lastCount = touch.getCount();
            } 
            if (global.lastTap != touch.getTap()) {
              sendOSC(settings.oscIP[0], settings.oscPORT[0], "tap", touch.getTap());
              sendOSC(settings.oscIP[1], settings.oscPORT[1], "tap",  touch.getTap());
              global.lastTap = touch.getTap();
            }
            if (global.lastDtap != touch.getDTap()) {
              sendOSC(settings.oscIP[0], settings.oscPORT[0], "dtap",  touch.getDTap());
              sendOSC(settings.oscIP[1], settings.oscPORT[1], "dtap",  touch.getDTap());
              global.lastDtap = touch.getDTap();
            }
            if (global.lastTtap != touch.getTTap()) {
              sendOSC(settings.oscIP[0], settings.oscPORT[0], "ttap",  touch.getTTap());
              sendOSC(settings.oscIP[1], settings.oscPORT[1], "ttap",  touch.getTTap());
              global.lastTtap = touch.getTTap();
            }
            if (global.lastDistance != ultData.distance) {
              sendOSC(settings.oscIP[0], settings.oscPORT[0], "ult", ultData.distance);
              sendOSC(settings.oscIP[1], settings.oscPORT[1], "ult", ultData.distance);
              global.lastDistance = ultData.distance;
            }
            if (global.lastTrig != ultData.trigger) {
              sendOSC(settings.oscIP[0], settings.oscPORT[0], "ultTrig", ultData.trigger);
              sendOSC(settings.oscIP[1], settings.oscPORT[1], "ultTrig", ultData.trigger);
              global.lastTrig = ultData.trigger;
            }
            if (global.lastJab[0] != instrument.getJabX() || global.lastJab[1] != instrument.getJabY() || global.lastJab[2] != instrument.getJabZ()) {
              sendTrioOSC(settings.oscIP[0], settings.oscPORT[0], "jab", instrument.getJabX(), instrument.getJabY(), instrument.getJabZ());
              sendTrioOSC(settings.oscIP[1], settings.oscPORT[1], "jab", instrument.getJabX(), instrument.getJabY(), instrument.getJabZ());
              global.lastJab[0] = instrument.getJabX(); global.lastJab[1] = instrument.getJabY(); global.lastJab[2] = instrument.getJabZ();
            }
            if (global.lastShake[0] != instrument.getShakeX() || global.lastShake[1] != instrument.getShakeY() || global.lastShake[2] != instrument.getShakeZ()) {
              sendTrioOSC(settings.oscIP[0], settings.oscPORT[0], "shake", instrument.getShakeX(), instrument.getShakeY(), instrument.getShakeZ());
              sendTrioOSC(settings.oscIP[1], settings.oscPORT[1], "shake", instrument.getShakeX(), instrument.getShakeY(), instrument.getShakeZ());
              global.lastShake[0] = instrument.getShakeX(); global.lastShake[1] = instrument.getShakeY(); global.lastShake[2] = instrument.getShakeZ();
            }
            if (battery.lastPercentage != battery.percentage) {
              sendOSC(settings.oscIP[0], settings.oscPORT[0], "battery", battery.percentage);
              sendOSC(settings.oscIP[1], settings.oscPORT[1], "battery", battery.percentage);
              battery.lastPercentage = battery.percentage;
            }
      }     
    #ifndef DISABLE_MIDI
    } else if (midi.status()) { // ... or MIDI
      // sending continuous data
      if (millis() - global.midiDelay > global.messageTimer) {
        global.messageTimer = millis();
        midi.CC (20, midi.mapMIDI(ultData.distance, 0, 300));   // CC 020 (0b00010100) (for the ultrasonic sensor)
        if (imu.dataAvailable()) {
          midi.CCbundle ( 21, midiReady.AccelX,// CC 021 (0b00010101) (for X-axis accelerometer)
                          22, midiReady.AccelY,// CC 022 (0b00010110) (for Y-axis accelerometer)
                          23, midiReady.AccelZ,// CC 023 (0b00010111) (for Z-axis accelerometer)
                          24, midiReady.GyroX, // CC 024 (0b00011000) (for X-axis gyroscope)
                          25, midiReady.GyroY, // CC 025 (0b00011001) (for Y-axis gyroscope)
                          26, midiReady.GyroZ, // CC 026 (0b00011010) (for Z-axis gyroscope)
                          27, midiReady.Yaw,   // CC 027 (0b00011011) (for yaw)
                          28, midiReady.Pitch, // CC 028 (0b00011100) (for pitch)
                          29, midiReady.Roll,  // CC 029 (0b00011101) (for roll)
                          85, midiReady.ShakeX,
                          86, midiReady.ShakeY,
                          87, midiReady.ShakeZ,
                          102, midiReady.JabX,
                          103, midiReady.JabY,
                          104, midiReady.JabZ);
        }
      }
      // send discrete (touch) data (only when it changes)
        if (global.lastCount != touch.getCount()) { // CC 15 (0b00001111) (for touch count)
          midi.CC (15, touch.getCount());   
          global.lastCount = touch.getCount();
        } 
        if (global.lastTap != touch.getTap()) {  // CC 30 (0b00011110) (for tap [0 or 127])
          if (touch.getTap() == 1) {
            midi.CC (30, 127);   
          } else {
            midi.CC (30, 0);
          }
          global.lastTap = touch.getTap();
        }
        if (global.lastDtap != touch.getDTap()) { // CC 31 (0b00011111) (for dtap [0 or 127])
          if (touch.getDTap() == 1) {
            midi.CC (31, 127);   
          } else {
            midi.CC (31, 0);
          }
          global.lastDtap = touch.getDTap();
        }
        if (global.lastTtap != touch.getTTap()) { // CC 14 (0b00001110) (for ttap [0 or 127])
          if (touch.getTTap() == 1) {
            midi.CC (14, 127);   
          } else {
            midi.CC (14, 0);
          }
          global.lastTtap = touch.getTTap();
        }
        if (global.lastTrig != ultData.trigger) { // CC 14 (0b00001110) (for ttap [0 or 127])
          if (ultData.trigger == 1) {
            midi.CC (89, 127);   
          } else {
            midi.CC (89, 0);
          }
          global.lastTrig = ultData.trigger;
        }  
    #endif
    }

  #ifndef DISABLE_LIBMAPPER
  // Update libmapper
    if (settings.libmapper && !global.midiFlag && WiFi.status() == WL_CONNECTED) {
      updateLibmapper();
    }
  #endif

  // receiving OSC
    if (!global.midiFlag) {
      receiveOSC();
    }

  // Check if setup mode has been called
    if (touch.getHold()) {
      settings.oldMode = settings.mode;
      settings.mode = 3;
      saveJSON();
      global.rebootFlag = true;
    }

  // Checking for timed reboot (called by setup mode) - reboots after 2 seconds
    if (global.rebootFlag && (millis() - 3000 > global.rebootTimer)) {
      ESP.restart();
    }

  // Set LED - connection status and battery level
  #ifdef board_WEMOS
    if (battery.percentage < 10) { // low battery - flickering
    led.setInterval(75);
    global.ledValue = led.blink(255, 50);
    ledcWrite(0, global.ledValue);
    } else {
      if (settings.mode == 3) { // 0:STA, 1:AP, and 3:Setup(STA+AP+WebServer)
        ledcWrite(0, 255); // stays always on in setup mode
      } else if (settings.mode == 0 || settings.mode == 1) { 
        if (WiFi.status() == WL_CONNECTED) { // blinks when connected, cycle when disconnected
          led.setInterval(1000);
          global.ledValue = led.blink(255, 40);
          ledcWrite(0, global.ledValue);
        } else {
          led.setInterval(4000);
          global.ledValue = led.cycle(global.ledValue, 0, 255);
          ledcWrite(0, global.ledValue);
        } 
      } else { // 2:MIDI
        if (midi.status()) { // blinks when connected, cycle when disconnected
          led.setInterval(1000);
          global.ledValue = led.blink(255, 40);
          ledcWrite(0, global.ledValue);
        } else {
          led.setInterval(4000);
          global.ledValue = led.cycle(global.ledValue, 0, 255);
          ledcWrite(0, global.ledValue);
        }
      }
    }
  #elif defined(board_TINYPICO)
    if (battery.percentage < 10) { // low battery (red)
      led.setInterval(20);
      global.color = led.blink(255, 20);
      tinypico.DotStar_SetPixelColor(global.color, 0, 0);
    } else {
      switch (settings.mode) { // 0:STA, 1:AP, 2:MIDI, 3:Setup(STA+AP+WebServer)
        case 0: // RGB: 0, 128, 255 (Dodger Blue)
          if (WiFi.status() == WL_CONNECTED) { // blinks when connected, cycle when disconnected
            led.setInterval(1000);
            global.color = led.blink(255,20);
            tinypico.DotStar_SetPixelColor(0, uint8_t(global.color/2), global.color);
          } else {
            led.setInterval(4000);
            global.color = led.cycle(global.color, 0, 255);
            tinypico.DotStar_SetPixelColor(0, uint8_t(global.color/2), global.color);
          }
          break;
        case 1: // RGB: 0, 255, 0 (Lime)
          if (WiFi.status() == WL_CONNECTED) { // blinks when connected, cycle when disconnected
            led.setInterval(1000);
            global.color = led.blink(255,20);
            tinypico.DotStar_SetPixelColor(0, global.color, 0);
          } else {
            led.setInterval(4000);
            global.color = led.cycle(global.color, 0, 255);
            tinypico.DotStar_SetPixelColor(0, global.color, 0);
          }
          break;
        case 2: // RGB: 255, 0, 255 (magenta)
          #ifndef DISABLE_MIDI
          if (midi.status()) { // blinks when connected, cycle when disconnected
            led.setInterval(1000);
            global.color = led.blink(255,20);
            tinypico.DotStar_SetPixelColor(global.color, 0, global.color);
          } else {
          #endif
            led.setInterval(4000);
            global.color = led.cycle(global.color, 0, 255);
            tinypico.DotStar_SetPixelColor(global.color, 0, global.color);
          #ifndef DISABLE_MIDI
          }
          #endif
          break;
        case 3: // RGB: 255, 128, 0 (dark orange)
            led.setInterval(1000);
            global.color = led.blink(255,20);
            tinypico.DotStar_SetPixelColor(global.color, uint8_t(global.color/2), 0);
          break;
        default: // if everything goes wrong it cycles through all colors (rainbow panic)
          tinypico.DotStar_CycleColor(25);
          break;
      }
    }
  #endif
} // end loop


///////////////
// Functions //
///////////////

void printVariables() {
  Serial.println("Printing loaded values (settings):");
  Serial.print("ID: "); Serial.println(settings.id);
  Serial.print("Designer: "); Serial.println(settings.author);
  Serial.print("Institution: "); Serial.println(settings.institution);
  Serial.print("AP password: "); Serial.println(settings.APpasswd);
  Serial.print("Saved SSID: "); Serial.println(settings.lastConnectedNetwork);
  Serial.print("Saved SSID password: "); Serial.println("********");
  Serial.print("Firmware version: "); Serial.println(settings.firmware);
  Serial.print("OSC IP #1: "); Serial.println(settings.oscIP[0]);
  Serial.print("OSC port #1: "); Serial.println(settings.oscPORT[0]);
  Serial.print("OSC IP #2: "); Serial.println(settings.oscIP[1]);
  Serial.print("OSC port #2: "); Serial.println(settings.oscPORT[1]);
  Serial.print("Local port: "); Serial.println(settings.localPORT);
  Serial.print("Libmapper mode: "); Serial.println(settings.libmapper);
  if (settings.mode == 3) {Serial.println("Setup mode enabled");} 
  else {Serial.print("Data transmission mode: "); Serial.println(settings.mode);}
  Serial.print("Stored mode: "); Serial.println(settings.oldMode);
  Serial.println();
}

struct SpiRamAllocator {
  void* allocate(size_t size) {
    return heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
  }
  void deallocate(void* pointer) {
    heap_caps_free(pointer);
  }
};

using SpiRamJsonDocument = BasicJsonDocument<SpiRamAllocator>;

void parseJSON() {    
  /* 
  Allocate a temporary JsonDocument
    Don't forget to change the capacity to match your requirements if using DynamicJsonDocument
    Use https://arduinojson.org/v6/assistant/ to compute the capacity.
    const size_t capacity = JSON_OBJECT_SIZE(15) + 250;
    DynamicJsonDocument doc(capacity);
  */
  SpiRamJsonDocument doc(1048576);

  if (SPIFFS.exists("/config.json")) { // file exists, reading and loading
    
    Serial.println("Reading config file...");
    File configFile = SPIFFS.open("/config.json", "r");
    
    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, configFile);
      if (error)
        Serial.println("Failed to read file!\n");
      else {
        // Copy values from the JsonDocument to the Config
        settings.id = doc["id"];
        strlcpy(settings.author, doc["author"], sizeof(settings.author));
        strlcpy(settings.institution, doc["institution"], sizeof(settings.institution));
        strlcpy(settings.APpasswd, doc["APpasswd"], sizeof(settings.APpasswd));
        strlcpy(settings.lastConnectedNetwork, doc["lastConnectedNetwork"], sizeof(settings.lastConnectedNetwork));
        strlcpy(settings.lastStoredPsk, doc["lastStoredPsk"], sizeof(settings.lastStoredPsk));
        settings.firmware = doc["firmware"];
        strlcpy(settings.oscIP[0], doc["oscIP1"], sizeof(settings.oscIP[0]));
        strlcpy(settings.oscIP[1], doc["oscIP2"], sizeof(settings.oscIP[1]));
        settings.oscPORT[0] = doc["oscPORT1"];
        settings.oscPORT[1] = doc["oscPORT2"];
        settings.localPORT = doc["localPORT"];
        settings.libmapper = doc["libmapper"];
        settings.osc = doc["osc"];
        settings.mode = doc["mode"];
        settings.oldMode = doc["oldMode"];
        settings.touchSensitivity = doc["touchSensitivity"];

        configFile.close();
        
        Serial.println("T-Stick configuration file loaded.\n");
      }
    } 
  else
    Serial.println("Failed to read config file!\n");
}

void saveJSON() { // Serializing
  
  Serial.println("Saving config to JSON file...");

  /*
  Allocate a temporary JsonDocument
    Don't forget to change the capacity to match your requirements if using DynamicJsonDocument
    Use https://arduinojson.org/v6/assistant/ to compute the capacity.
    
    const size_t capacity = JSON_OBJECT_SIZE(15);
    DynamicJsonDocument doc(capacity);
  */

  SpiRamJsonDocument doc(1048576);

  // Copy values from Config to the JsonDocument
    doc["id"] = settings.id;
    doc["author"] = settings.author;
    doc["institution"] = settings.institution;
    doc["APpasswd"] = settings.APpasswd;
    doc["lastConnectedNetwork"] = settings.lastConnectedNetwork;
    doc["lastStoredPsk"] = settings.lastStoredPsk;
    doc["firmware"] = settings.firmware;
    doc["oscIP1"] = settings.oscIP[0];
    doc["oscPORT1"] = settings.oscPORT[0];
    doc["oscIP2"] = settings.oscIP[1];
    doc["oscPORT2"] = settings.oscPORT[1];
    doc["localPORT"] = settings.localPORT;
    doc["libmapper"] = settings.libmapper;
    doc["osc"] = settings.osc;
    doc["mode"] = settings.mode;
    doc["oldMode"] = settings.oldMode;
    doc["touchSensitivity"] = settings.touchSensitivity;
 
  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {Serial.println("Failed to open config file for writing!\n");}
  
  // Serialize JSON to file
    if (serializeJson(doc, configFile) == 0)
      Serial.println("Failed to write to file");
    else
      Serial.println("JSON file successfully saved!\n");
  
  configFile.close();
} //end save

////////////////
// Ultrasonic //
////////////////


void ultFilter() {
  ultData.filterArray.push_back(ultData.distance);
  if(ultData.filterArray.size() > ultData.queueAmount) {
    ultData.filterArray.pop_front();
  }
  ultData.distance = 0;
  for (int i=0; i<ultData.filterArray.size(); i++) {
    ultData.distance += ultData.filterArray.at(i);
  }
  ultData.distance /= ultData.filterArray.size();
}

void readUltTrigger() {
    if (millis() - ultData.interval > ultData.timer) {
        ultData.lastDistance = ultData.tempDistance;
        ultData.tempDistance = ult.ping_mm();
        if (ultData.tempDistance < ultData.ultMinDistance) {
            ultData.tempDistance = 0;
        }
        if (ultData.lastDistance == 0) {
            ultData.trigTimer = millis();
            if (ultData.tempDistance == 0) {
                ultData.distance = 0;
                ultData.trigger = 0;
            } 
        } else {
            if (millis() - ultData.trigInterval > ultData.trigTimer) {
                ultData.distance = ultData.tempDistance;
                ultData.trigger = 0;
            } else if (ultData.tempDistance == 0 && ultData.distance != 0) {
                ultData.trigger = 1;
            }
        }
        ultData.timer = millis();
    }
}


void readUlt() {
    if (millis() - ultData.interval > ultData.timer) {
        ultData.tempDistance = ult.ping_mm();
        if (ultData.tempDistance < ultData.ultMinDistance) {
            ultData.distance = 0;
        } else {
            ultData.distance = ultData.tempDistance;
        }
        ultFilter();
        ultData.timer = millis();
    }
}
    

/////////////
// Battery //
/////////////

  // read battery level (based on https://www.youtube.com/watch?v=yZjpYmWVLh8&feature=youtu.be&t=88) 
  void readBattery() {
    #ifdef board_WEMOS
      battery.value = analogRead(pin.battery) / 4096.0 * 7.445;
    #elif defined(board_TINYPICO)
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


//////////
// Site //
//////////

String indexProcessor(const String& var) {
  if(var == "DEVICENAME")
    return global.deviceName;
  if(var == "STATUS") {
    if (WiFi.status() != WL_CONNECTED) {
      char str[40]; 
      snprintf(str, sizeof(str), "Currently not connected to any network");
      return str;
    } else {
    char str[120];
    snprintf (str, sizeof(str), "Currently connected on <strong style=\"color:Tomato;\">%s</strong> network (IP: %s)", settings.lastConnectedNetwork, WiFi.localIP().toString().c_str());
    return str;
    }
  }
  if(var == "CURRENTSSID")
    return WiFi.SSID();
  if(var == "CURRENTPSK")
    return settings.lastStoredPsk;
  if(var == "MODE0") {
    if(settings.oldMode == 0) {
      return "selected";
    } else {
      return "";
    }
  }
  if(var == "MODE1") {
    if(settings.oldMode == 1) {
      return "selected";
    } else {
      return "";
    }
  }
  if(var == "MODE2") {
    if(settings.oldMode == 2) {
      return "selected";
    } else {
      return "";
    }
  }
  if(var == "CURRENTIP")
    return WiFi.localIP().toString();
  if(var == "CURRENTAPIP")
    return WiFi.softAPIP().toString();
  if(var == "CURRENTSTAMAC")
      return WiFi.macAddress();
  if(var == "CURRENTAPMAC")
      return WiFi.softAPmacAddress();
  if(var == "CURRENTOSC1")
      return settings.oscIP[0];
  if(var == "CURRENTOSC2")
      return settings.oscIP[1]; 
  if(var == "CURRENTLM") {
    if(settings.libmapper) {
      return "checked";
    } else {
      return "";
    }
  }
  if(var == "CURRENTOSC") {
    if(settings.osc) {
      return "checked";
    } else {
      return "";
    }
  }
  if(var == "CURRENTPORT1") {
    char str[7]; 
    snprintf(str, sizeof(str), "%d", settings.oscPORT[0]);
    return str;
  }
  if(var == "CURRENTPORT2") {
    char str[7]; 
    snprintf(str, sizeof(str), "%d", settings.oscPORT[1]);
    return str;
  }
  if(var == "TSTICKID") {
      char str[4];
      snprintf (str, sizeof(str), "%03d", settings.id);
      return str;
  }
  if(var == "TSTICKAUTH")
      return settings.author;
  if(var == "TSTICKINST")
      return settings.institution;
  if(var == "TSTICKVER")
      return String(settings.firmware);
    
  return String();
}

String factoryProcessor(const String& var) {
  if(var == "DEVICENAME")
    return global.deviceName;
  if(var == "STATUS") {
    if (WiFi.status() != WL_CONNECTED) {
      char str[40]; 
      snprintf(str, sizeof(str), "Currently not connected to any network");
      return str;
    } else {
    char str[120];
    snprintf (str, sizeof(str), "Currently connected on <strong style=\"color:Tomato;\">%s</strong> network (IP: %s)", settings.lastConnectedNetwork, WiFi.localIP().toString().c_str());
    return str;
    }
  }
  if(var == "TSTICKID") {
      char str[4];
      snprintf (str, sizeof(str), "%03d", settings.id);
      return str;
  }
  if(var == "TSTICKVER")
      return String(settings.firmware);
  if(var == "TOUCH")
      return String(settings.touchSensitivity);

  return String();
}

String scanProcessor(const String& var) {
  if(var == "SSIDS")
    return module.wifiScanResults;
    
  return String();
}

String updateProcessor(const String& var) {
  if(var == "UPDATESTATUSF")
    return module.wifiScanResults;
  if(var == "UPDATESTATUSS")
    return module.wifiScanResults;
    
  return String();
}

void initWebServer() {

  // Route for root page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(SPIFFS, "/index.html", String(), false, indexProcessor);
    });
    server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
      printf("\nSettings received! (HTTP_post):\n");
      if(request->hasParam("reboot", true)) {
          request->send(SPIFFS, "/reboot.html");
          if (settings.mode == 3)
            settings.mode = settings.oldMode;
          global.rebootFlag = true;
          global.rebootTimer = millis();
      } else {
        if(request->hasParam("SSID", true)) {
            //Serial.println("SSID received");
            strcpy(settings.lastConnectedNetwork, request->getParam("SSID", true)->value().c_str());
            printf("SSID stored: %s\n", settings.lastConnectedNetwork);
        } else {
            printf("No SSID received\n");
        }
        if(request->hasParam("password", true)) {
            //Serial.println("SSID Password received");
            strcpy(settings.lastStoredPsk, request->getParam("password", true)->value().c_str());
            printf("SSID password stored: ********\n");
        } else {
            printf("No SSID password received\n");
        }
        if(request->hasParam("APpasswd", true) && request->hasParam("APpasswdValidate", true)) {
            if(request->getParam("APpasswd", true)->value() == request->getParam("APpasswdValidate", true)->value() && request->getParam("APpasswd", true)->value() != "") {
              strcpy(settings.APpasswd, request->getParam("APpasswd", true)->value().c_str());
              printf("AP password stored: %s\n", settings.APpasswd);
            } else {
              printf("AP password blank or not match retype. Keeping old password\n");
            }
        } else {
            printf("No AP password received\n");
        }
        if(request->hasParam("mode", true)) {
            settings.mode = atoi(request->getParam("mode", true)->value().c_str());
            settings.oldMode = settings.mode;
            printf("T-Stick mode stored: %d\n", settings.mode);
        } else {
            printf("No mode received");
        }
        if(request->hasParam("oscIP1", true)) {
            if(request->getParam("oscIP1", true)->value() == ""){
              strcpy(settings.oscIP[0],"0.0.0.0");
            } else {
              strcpy(settings.oscIP[0], request->getParam("oscIP1", true)->value().c_str());
            }
            printf("Primary IP received: %s\n", settings.oscIP[0]);
        } else {
            printf("No Primary IP received\n");
        }
        if(request->hasParam("oscPORT1", true)) {
            settings.oscPORT[0] = atoi(request->getParam("oscPORT1", true)->value().c_str());
            printf("Primary port received: %d\n", settings.oscPORT[0]);
        } else {
            printf("No Primary port received\n");
        }
        if(request->hasParam("oscIP2", true)) {
          if(request->getParam("oscIP2", true)->value() == ""){
              strcpy(settings.oscIP[1],"0.0.0.0");
            } else {
              strcpy(settings.oscIP[1], request->getParam("oscIP2", true)->value().c_str());
            }
        } else {
            printf("No Secondary IP received\n");
        }
        if(request->hasParam("oscPORT2", true)) {
            settings.oscPORT[1] = atoi(request->getParam("oscPORT2", true)->value().c_str());
            printf("Secondary port received: %d\n", settings.oscPORT[0]);
        } else {
            printf("No Secondary port received\n");
        }
        if(request->hasParam("libmapper", true)) {
            printf("Libmapper TRUE received\n");
            settings.libmapper = true;
        } else {
            printf("Libmapper FALSE received\n");
            settings.libmapper = false;
        }
        if(request->hasParam("osc", true)) {
            printf("OSC TRUE received\n");
            settings.osc = true;
        } else {
            printf("OSC FALSE received\n");
            settings.osc = false;
        }
        request->send(SPIFFS, "/index.html", String(), false, indexProcessor);
        //request->send(200);
      }
      if (settings.mode == 3)
        settings.mode = settings.oldMode;
      saveJSON();
    });

  // Route for scan page
    server.on("/scan", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(SPIFFS, "/scan.html", String(), false, scanProcessor);
    });

  // Route for factory page
    server.on("/factory", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(SPIFFS, "/factory.html", String(), false, factoryProcessor);
    });
    server.on("/factory", HTTP_POST, [](AsyncWebServerRequest *request) {
        printf("\nFactory Settings received! (HTTP_post):\n");
        if(request->hasParam("reboot", true)) {
            request->send(SPIFFS, "/reboot.html");
            settings.mode = 3;
            global.rebootFlag = true;
            global.rebootTimer = millis();
        } else {
          if(request->hasParam("ID", true)) {
              settings.id = atoi(request->getParam("ID", true)->value().c_str());
              printf("ID (factory) received: %d\n", settings.id);
          } else {
              printf("No ID (factory) received\n");
          }
          if(request->hasParam("firmware", true)) {
              settings.firmware = atoi(request->getParam("firmware", true)->value().c_str());
              printf("Firmware # (factory) received: %d\n", settings.firmware);
          } else {
              printf("No Firmware # (factory) received\n");
          }
          if(request->hasParam("touch", true)) {
            settings.touchSensitivity = atoi(request->getParam("touch", true)->value().c_str());
              touch.setSensitivity(settings.touchSensitivity);
              printf("Touch sensitivity (factory) received: %d\n", touch.getThreshold());
          } else {
              printf("No touch sensitivity (factory) received\n");
          }
          request->send(SPIFFS, "/factory.html", String(), false, factoryProcessor);
          //request->send(200);
        }
        if (settings.mode == 3)
          settings.mode = settings.oldMode;
        saveJSON();
      });

  // Route for update page
    server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(SPIFFS, "/update.html");
    });
    /*handling uploading firmware file */
    server.on("/updateF", HTTP_POST, [](AsyncWebServerRequest *request){
      global.rebootFlag = !Update.hasError();
      AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", global.rebootFlag?"Update complete, rebooting":"Fail to send file and/or update");
      response->addHeader("Connection", "close");
      request->send(response);
    },[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
      if(!index){
        Serial.printf("Firmware Update Start: %s\n", filename.c_str());
        //Update.runAsync(true);
        if(!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000)){
          Update.printError(Serial);
        }
      }
      if(!Update.hasError()){
        if(Update.write(data, len) != len){
          Update.printError(Serial);
        }
      }
      if(final){
        if(Update.end(true)){
          Serial.printf("Firmware Update Success: %uB\n", index+len);
        } else {
          Update.printError(Serial);
        }
      }
    });     
    /*handling uploading SPIFFS file */
    server.on("/updateS", HTTP_POST, [](AsyncWebServerRequest *request){
      global.rebootFlag = !Update.hasError();
      AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", global.rebootFlag?"Update complete, rebooting":"Fail to send file and/or update");
      response->addHeader("Connection", "close");
      request->send(response);
    },[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
      if(!index){
        Serial.printf("SPIFFS Update Start: %s\n", filename.c_str());
        //Update.runAsync(true);
        if(!Update.begin(UPDATE_SIZE_UNKNOWN, U_SPIFFS)){
          Update.printError(Serial);
        }
      }
      if(!Update.hasError()){
        if(Update.write(data, len) != len){
          Update.printError(Serial);
        }
      }
      if(final){
        if(Update.end(true)){
          Serial.printf("SPIFFS Update Success: %uB\n", index+len);
        } else {
          Update.printError(Serial);
        }
      }
    });  

  // Route to load style.css file
    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(SPIFFS, "/style.css", "text/css");
    });

  // Route to documentation
    server.on("/doc", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(SPIFFS, "/docs/T-Stick_module.html");
    });
    server.on("/T-Stick_module.md", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(SPIFFS, "/docs/T-Stick_module.md", String(), true);
    });
    server.on("/T-Stick_module.pd", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(SPIFFS, "/docs/T-Stick_module.pd", String(), true);
    });

  server.begin();
  
  Serial.println("HTTP server started");
}

void start_mdns_service() {
    //initialize mDNS service
    esp_err_t err = mdns_init();
    if (err) {
        printf("MDNS Init failed: %d\n", err);
        return;
    }

    //set hostname
    mdns_hostname_set(global.deviceName);
    //set default instance
    mdns_instance_name_set(global.deviceName);
}

/////////
// OSC //
/////////

 void sendContinuousOSC(char* ip,int32_t port) {

  IPAddress oscIP;
  IPAddress emptyIP(0,0,0,0);
  
  if (oscIP.fromString(ip) && oscIP != emptyIP) {
    char namespaceBuffer[40];

    static OSCBundle continuous;

    // snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/%s/ult",global.deviceName);
    // OSCMessage msgult(namespaceBuffer);
    //   msgult.add(ultData.distance);
    //   msgult.add(ultData.trigger);
    //   oscEndpoint.beginPacket(oscIP,port);
    //   msgult.send(oscEndpoint);
    //   oscEndpoint.endPacket();
    //   msgult.empty(); 

    if (imu.dataAvailable()) {
        snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/%s/accl",global.deviceName);
        OSCMessage msgAccl(namespaceBuffer);
          msgAccl.add(imu.getAccelX());
          msgAccl.add(imu.getAccelY());
          msgAccl.add(imu.getAccelZ());
          continuous.add(msgAccl);
      
        snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/%s/gyro",global.deviceName);
        OSCMessage msgGyro(namespaceBuffer);
          msgGyro.add(imu.getGyroX());
          msgGyro.add(imu.getGyroY());
          msgGyro.add(imu.getGyroZ());
          continuous.add(msgGyro);
      
        snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/%s/magn",global.deviceName);
        OSCMessage msgMagn(namespaceBuffer);
          msgMagn.add(imu.getMagX());
          msgMagn.add(imu.getMagY());
          msgMagn.add(imu.getMagZ());
          continuous.add(msgMagn);

        oscEndpoint.beginPacket(oscIP,port);
        continuous.send(oscEndpoint);
        oscEndpoint.endPacket();
        continuous.empty(); 

        snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/%s/quat",global.deviceName);
        OSCMessage msgQuat(namespaceBuffer);
          msgQuat.add(imu.getQuatI());
          msgQuat.add(imu.getQuatJ());
          msgQuat.add(imu.getQuatK());
          msgQuat.add(imu.getQuatReal());
          oscEndpoint.beginPacket(oscIP,port);
          msgQuat.send(oscEndpoint);
          oscEndpoint.endPacket();
          msgQuat.empty(); 

        snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/%s/ypr",global.deviceName);
        OSCMessage msgEuler(namespaceBuffer);
          msgEuler.add(imu.getYaw());
          msgEuler.add(imu.getPitch());
          msgEuler.add(imu.getRoll());
          oscEndpoint.beginPacket(oscIP,port);
          msgEuler.send(oscEndpoint);
          oscEndpoint.endPacket();
          msgEuler.empty(); 

        // snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/%s/shake",global.deviceName);
        // OSCMessage msgShake(namespaceBuffer);
        //   msgShake.add(instrument.getShakeX());
        //   msgShake.add(instrument.getShakeY());
        //   msgShake.add(instrument.getShakeZ());
        //   oscEndpoint.beginPacket(oscIP,port);
        //   msgShake.send(oscEndpoint);
        //   oscEndpoint.endPacket();
        //   msgShake.empty(); 

        // snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/%s/jab",global.deviceName);
        // OSCMessage msgJab(namespaceBuffer);
        //   msgJab.add(instrument.getJabX());
        //   msgJab.add(instrument.getJabY());
        //   msgJab.add(instrument.getJabZ());
        //   oscEndpoint.beginPacket(oscIP,port);
        //   msgJab.send(oscEndpoint);
        //   oscEndpoint.endPacket();
        //   msgJab.empty();

        snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/%s/touch",global.deviceName);
        OSCMessage msgTouch(namespaceBuffer);
          msgTouch.add(touch.getValue());
          oscEndpoint.beginPacket(oscIP,port);
          msgTouch.send(oscEndpoint);
          oscEndpoint.endPacket();
          msgTouch.empty();
    }
  }
}

void sendOSC(char* ip,int32_t port, const char* messageNamespace, float data) {
    IPAddress oscIP;
    IPAddress emptyIP(0,0,0,0);
    if (oscIP.fromString(ip) && oscIP != emptyIP) {
      char namespaceBuffer[40];
      snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/%s/%s",global.deviceName, messageNamespace);
      OSCMessage msg(namespaceBuffer);
      msg.add(data);
      oscEndpoint.beginPacket(oscIP, port);
      msg.send(oscEndpoint);
      oscEndpoint.endPacket();
      msg.empty();
  }
}

void sendTrioOSC(char* ip,int32_t port, const char* messageNamespace, float data1, float data2, float data3) {
    IPAddress oscIP;
    IPAddress emptyIP(0,0,0,0);
    if (oscIP.fromString(ip) && oscIP != emptyIP) {
      char namespaceBuffer[40];
      snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/%s/%s",global.deviceName, messageNamespace);
      OSCMessage msg(namespaceBuffer);
      msg.add(data1);
      msg.add(data2);
      msg.add(data3);
      oscEndpoint.beginPacket(oscIP, port);
      msg.send(oscEndpoint);
      oscEndpoint.endPacket();
      msg.empty();
  }
}

void sendInfo(OSCMessage &msg) {
  // Send back instrument's current config
  char namespaceBuffer[30];
  snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/T-Stick_%i/info",settings.id);
  IPAddress oscIP;  
  if (oscIP.fromString(settings.oscIP[0])) {
    OSCMessage msgInfo(namespaceBuffer);
    msgInfo.add(settings.id);
    msgInfo.add(settings.firmware);
    oscEndpoint.beginPacket(oscIP, settings.oscPORT[0]);
    msgInfo.send(oscEndpoint);
    oscEndpoint.endPacket();
    msgInfo.empty();
  }
  if (oscIP.fromString(settings.oscIP[1])) {
    OSCMessage msgInfo(namespaceBuffer);
    msgInfo.add(settings.id);
    msgInfo.add(settings.firmware);
    oscEndpoint.beginPacket(oscIP, settings.oscPORT[1]);
    msgInfo.send(oscEndpoint);
    oscEndpoint.endPacket();
    msgInfo.empty();
  }
}

void receiveOSC() {

  OSCErrorCode error;
  OSCMessage msgReceive;
  int size = oscEndpoint.parsePacket();

  if (size > 0) {
    Serial.println("\nOSC message received");
    while (size--) {
      msgReceive.fill(oscEndpoint.read());
    }
    if (!msgReceive.hasError()) {
      Serial.println("Routing OSC message...");
      msgReceive.dispatch("/state/info", sendInfo); // send back instrument's current config
      //msgReceive.dispatch("/state/setup", openPortalOSC); // open portal
    } else {
      error = msgReceive.getError();
      Serial.print("\nOSC receive error: "); Serial.println(error);
    }
  }
}

///////////////
// LIBMAPPER //
///////////////

#ifndef DISABLE_LIBMAPPER

    void initLibmapper() {

    dev = new mapper::Device(global.deviceName);
    
        //lm.ult = dev->add_signal(mapper::Direction::OUTGOING, "ult", 1, mapper::Type::INT32, "mm", &lm.ultMin, &lm.ultMax);
        // lm.accel = dev.add_signal(mapper::Direction::OUTGOING, "accel", 3, mapper::Type::FLOAT, "m/s^2", &lm.accelMin, &lm.accelMax);
        // lm.gyro = dev.add_signal(mapper::Direction::OUTGOING, "gyro", 3, mapper::Type::FLOAT, "rad/s", &lm.gyroMin, &lm.gyroMax);
        // lm.mag = dev.add_signal(mapper::Direction::OUTGOING, "mag", 3, mapper::Type::FLOAT, "uTesla", &lm.magMin, &lm.magMax);
        // lm.quat = dev.add_signal(mapper::Direction::OUTGOING, "quat", 4, mapper::Type::FLOAT, NULL, &lm.quatMin, &lm.quatMax);
        // lm.euler = dev.add_signal(mapper::Direction::OUTGOING, "euler", 3, mapper::Type::FLOAT, "degrees", &lm.eulerMin, &lm.eulerMax);
        // lm.shake = dev.add_signal(mapper::Direction::OUTGOING, "shake", 3, mapper::Type::FLOAT, NULL, &lm.shakeMin, &lm.shakeMax);
        // lm.jab = dev.add_signal(mapper::Direction::OUTGOING, "jab", 3, mapper::Type::FLOAT, NULL, &lm.jabMin, &lm.jabMax);
        // lm.count = dev.add_signal(mapper::Direction::OUTGOING, "count", 1, mapper::Type::INT32, NULL, &lm.countMin, &lm.countMax);
        // lm.tap = dev.add_signal(mapper::Direction::OUTGOING, "tap", 1, mapper::Type::INT32, NULL, &lm.tapMin, &lm.tapMax);
        // lm.dtap = dev.add_signal(mapper::Direction::OUTGOING, "dtap", 1, mapper::Type::INT32, NULL, &lm.tapMin, &lm.tapMax);
        // lm.ttap = dev.add_signal(mapper::Direction::OUTGOING, "ttap", 1, mapper::Type::INT32, NULL, &lm.tapMin, &lm.tapMax);
        // lm.bat = dev.add_signal(mapper::Direction::OUTGOING, "battery", 1, mapper::Type::INT32, NULL, &lm.batMin, &lm.batMax);
    }

    void updateLibmapper() {
        
        Serial.println(dev->ready());
        Serial.println(dev->graph().iface().c_str());



        dev->poll();

        // int libUlt = ultData.distance;
        // lm.ult.set_value(libUlt);
        // float libAccel[3] = {imu.getAccelX(),imu.getAccelY(),imu.getAccelZ()};
        // lm.accel.set_value(libAccel);
        // float libGyro[3] = {imu.getGyroX(),imu.getGyroY(),imu.getGyroZ()};
        // lm.gyro.set_value(libGyro);
        // float libMag[3] = {imu.getMagX(),imu.getMagY(),imu.getMagZ()};
        // lm.mag.set_value(libMag);
        // float libQuat[4] = {imu.getQuatI(), imu.getQuatJ(), imu.getQuatK(), imu.getQuatReal()};
        // lm.quat.set_value(libQuat);
        // float libEuler[3] = {imu.getYaw(),imu.getPitch(), imu.getRoll()};
        // lm.euler.set_value(libEuler);
        // float libShake[3] = {instrument.getShakeX(), instrument.getShakeY(), instrument.getShakeZ()};
        // lm.shake.set_value(libShake);
        // float libJab[3] = {instrument.getJabX(), instrument.getJabY(), instrument.getJabZ()};
        // lm.jab.set_value(libJab);
        // int libCount = touch.getCount();
        // lm.count.set_value(libCount);
        // int libTap = touch.getTap();
        // lm.tap.set_value(libTap);
        // int libDTap = touch.getDTap();
        // lm.dtap.set_value(libDTap);
        // int libTTap = touch.getTTap();
        // lm.ttap.set_value(libTTap);
        // int libBat = battery.percentage;
        // lm.bat.set_value(libBat);
  }

#endif