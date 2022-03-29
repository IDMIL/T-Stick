
//****************************************************************************//
// T-Stick                                                                    //
// Input Devices and Music Interaction Laboratory (IDMIL), McGill University  //
// Edu Meneses (2022) - https://www.edumeneses.com                            //
//****************************************************************************//

/*
OBS: There's an error with the ESP32 inet.h file. The file requires manual fixing. Check the issue at 
https://github.com/mathiasbredholt/libmapper-arduino/issues/3 
*/


//
// ╺┳┓┏━╸┏━┓┏━┓
//  ┃┃┣╸ ┣━ ┗━┓
// ╺┻┛┗━╸╹  ┗━┛
//

const unsigned int firmware_version = 220222;

/* 
  Choose proper microcontroller:
  - TinyPICO (3GW) 
  - Wemos D32 Pro
*/
#define board_TINYPICO
// #define board_WEMOS

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

//////////////
// Includes //
//////////////

#include <Arduino.h>
#include <Update.h>       // For OTA over web server (manual .bin upload)

#include <deque>
#include <cmath>          // std::abs
#include <algorithm>      // std::min_element, std::max_element

#include "mdns.h"

#include <OSCMessage.h>   // https://github.com/CNMAT/OSC
#include <OSCBundle.h>


#include <mapper_cpp.h> //libmapper library
#include <string>


#ifdef board_TINYPICO
    #include <TinyPICO.h>
    TinyPICO tinypico = TinyPICO();
#endif

/////////////////////
// Pin definitions //
/////////////////////

  struct Pin {
        byte led = 5;      // Built In LED pin
        byte battery = 35; // To check battery level (voltage)
        byte fsr = 33;
        byte button = 15;
    } pin;


///////////////////////////////
// Firmware global variables //
///////////////////////////////

  struct global {
    char deviceName[25];
    char APpasswd[15];
    bool rebootFlag = false;
    unsigned long rebootTimer;
    unsigned int oscDelay = 10; // 10ms ~= 100Hz
    unsigned long messageTimer = 0;
    unsigned int lastCount = 1;
    unsigned int lastTap = 1;
    unsigned int lastDtap = 1;
    unsigned int lastTtap = 1;
    unsigned int lastTrig = 1;
    unsigned int lastDistance = 1;
    float lastFsr = 1;
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
    char author[20];
    char institution[20];
    char APpasswd[15];
    char lastConnectedNetwork[30];
    char lastStoredPsk[30];
    int32_t firmware;
    char oscIP[2][17] = { {'0','0','0','0'}, {'0','0','0','0'} };
    int32_t oscPORT[2] = {8000, 8000};
    int32_t localPORT = 8000;
    bool libmapper = false;
    bool osc = false;
    int mode = 0; // 0:STA, 1:Setup(STA+AP+WebServer)
    int fsrOffset;
  } settings;
  
mapper::Device* dev;

struct Lm {
  int rawCapsenseMin = 0, rawCapsenseMax = 1;
  float rawGyroMin = -34.90659, rawGyroMax = 34.90659;
  int rawAcclMin = -32767, rawAcclMax = 32767;
  int rawMagnMin = -32767, rawMagnMax = 32767;
  int rawFSRMin = 0, rawFSRMax = 4095;
  int rawPiezoMin = 0, rawPiezoMax = 1023;
  float orientationMin = -1.0, orientationMax = 1.0;
  float magMin = 0.0, magMax = 1.7320508;  // sqrt(3)
  int buttonMin = 0, buttonMax = 1;
  float yprMin = -180.0, yprMax = 180.0;
  float instTouchMin = 0.0, instTouchMax = 1.0;
  float genericMin = 0.0, genericMax = 100.0;
  mapper::Signal sigRawCapsense;
  mapper::Signal sigRawGyroX;
  mapper::Signal sigRawGyroY;
  mapper::Signal sigRawGyroZ;
  mapper::Signal sigRawAcclX;
  mapper::Signal sigRawAcclY;
  mapper::Signal sigRawAcclZ;
  mapper::Signal sigRawMagnX;
  mapper::Signal sigRawMagnY;
  mapper::Signal sigRawMagnZ;
  mapper::Signal sigRawFSR;
  mapper::Signal sigRawPiezo;
  mapper::Signal sigOrientationQ1;
  mapper::Signal sigOrientationQ2;
  mapper::Signal sigOrientationQ3;
  mapper::Signal sigOrientationQ4;
  mapper::Signal sigYaw;
  mapper::Signal sigPitch;
  mapper::Signal sigRoll;
  mapper::Signal sigMagGyro;
  mapper::Signal sigMagAccl;
  mapper::Signal sigMagMagn;
  mapper::Signal sigButton;
  mapper::Signal sigLongButton;
  mapper::Signal sigDoubleButton;
  mapper::Signal sigtouchAll;
  mapper::Signal sigtouchTop;
  mapper::Signal sigtouchMiddle;
  mapper::Signal sigtouchBottom;
  mapper::Signal sigBrush;
  mapper::Signal sigRub;
  mapper::Signal sigMultiBrush;
  mapper::Signal sigMultiRub;
  mapper::Signal sigShakeXYZ;
  mapper::Signal sigJabXYZ;
} lm;

  void initLibmapper();
  void updateLibmapper();
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

  #include "button.h"

  Button button;

////////////////////////////////
// Include IMU function files //
////////////////////////////////

  #include "lsm9ds1.h"

  Imu_LSM9DS1 imu;

////////////////////////////////
// Include FSR function files //
////////////////////////////////

  #include "fsr.h"

  Fsr fsr;

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

//////////////////////////////////////////////
// Include Touch stuff                      //
//////////////////////////////////////////////

#ifdef touch_TRILL
  #include "touch.h"
  #include "instrument_touch.h"
  Touch touch;
#endif

#ifdef touch_CAPSENSE
  #include "capsense.h"
  #include "instrument_touch.h"
  Capsense capsense;
#endif

Instrument_touch instrument_touch;

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

//
// ┏━┓┏━╸╺┳╸╻ ╻┏━┓
// ┗━┓┣╸  ┃ ┃ ┃┣━┛
// ┗━┛┗━╸ ╹ ┗━┛╹  
// 

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
    if (settings.mode < 0 || settings.mode > 1 ){
      settings.mode = 1;
      printf("\nMode error: changing to setup mode for correction\n");
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
    snprintf(global.deviceName,(sizeof(global.deviceName)-1),"T-Stick_%03i",settings.id);

    module.startWifi(global.deviceName, settings.mode, settings.APpasswd, settings.lastConnectedNetwork, settings.lastStoredPsk);

  // Start listening for incoming OSC messages if WiFi is ON
    oscEndpoint.begin(settings.localPORT);
    printf("Starting UDP (listening to OSC messages)\n");
    printf("Local port: %d\n", settings.localPORT);


  // Initializing button (set pin)
    printf("    Initializing button configuration...  ");
    if (button.initButton(pin.button)) {
      printf("done\n");
    } else {
      printf("Button initialization failed!\n");
    }

  // Initializing IMU
    printf("    Initializing IMU...  ");
    if (imu.initIMU()) {
      printf("done\n");
    } else {
      printf("IMU initialization failed!\n");
    }

  // Initializing FSR
    printf("    Initializing FSR...  ");
    if (fsr.initFsr(pin.fsr, settings.fsrOffset)) {
      printf("done\n");
    } else {
      printf("FSR initialization failed!\n");
    }

  // Initializing capacitive sensing
  #ifdef touch_TRILL
    printf("    Initializing Touch stuff...  ");
    if (touch.initTouch()) {
      printf("done\n");
    } else {
      printf("Touch initialization failed!\n");
    }
  #endif

  #ifdef touch_CAPSENSE
    capsense.capsense_scan(); // Look for Capsense boards and return their addresses
                              // must run before initLibmapper to get # of capsense boards
  #endif

  #ifdef LIBMAPPER
    if (settings.libmapper == 1) {
      Serial.println("\nStarting libmapper...");
      initLibmapper();
      Serial.println("Done\n");
    }
  #endif

  // LED set pin
    led.setPin(pin.led);

  // Start dns and web Server if in setup mode
    if (settings.mode == 1) {
      printf("Starting DNS server\n");
      dnsServer.start(53, "*", WiFi.softAPIP());
      start_mdns_service();
      initWebServer();
    }

  // Setting Deep sleep wake button
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_15,0); // 1 = High, 0 = Low
    
  printf("\n\nBoot complete\n\n"
          "This firmware accepts:\n"
          "- 's' to start setup mode\n"
          "- 'r' to reboot\n"
          "- 'd' to enter deep sleep\n\n");

} // end Setup


//
// ╻  ┏━┓┏━┓┏━┓
// ┃  ┃ ┃┃ ┃┣━┛
// ┗━╸┗━┛┗━┛╹  
//

void loop() {

  // Check for serial messages. This firmware accepts:
  //     - 's' to start setup mode
  //     - 'r' to reboot
  //     - 'd' to enter deep sleep
  if (Serial.available() > 0) {
    int incomingByte = 0;
    incomingByte = Serial.read();
    switch(incomingByte) {
      case 114: // r
        printf("\nRebooting...\n");
        global.rebootFlag = true;
        break;
      case 115: // s
        printf("\nEntering setup mode\n");
        settings.mode = 1;
        saveJSON();
        global.rebootFlag = true;
        break;
      case 100: // d
        printf("\nEntering deep sleep.\n\nGoodbye!\n");
        delay(1000);
        esp_deep_sleep_start();
        break;
      case 10:
        // ignore LF
        break;
      case 13:
        // ignore CR
        break;
      default:
        printf("\nI don't recognize this command\n"
          "This firmware accepts:\n"
            "'s' to start setup mode\n"
            "'r' to reboot\n"
            "'d' to enter deep sleep\n\n");
    }
  }

  // go to deep sleep if double press button
  if (button.getDTap()){
      printf("\nEntering deep sleep.\n\nGoodbye!\n");
      delay(1000);
      esp_deep_sleep_start();
  }

  if (settings.mode == 1) {
    dnsServer.processNextRequest();
  }

  // Read button
    button.readButton();

  // Read FSR
    fsr.readFsr();

  // update libmapper
  #ifdef LIBMAPPER
    if (settings.libmapper == 1) {
      updateLibmapper();
    }
  #endif

  // Read Touch
  #ifdef touch_TRILL
    touch.readTouch();
    touch.cookData();
    instrument_touch.updateInstrument(touch.touch,touch.touchSize);
  #endif
  #ifdef touch_CAPSENSE
    capsense.readCapsense();
    instrument_touch.updateInstrument(capsense.data,capsense.touchStripsSize);
  #endif

  // read battery
    if (millis() - battery.interval > battery.timer) {
      battery.timer = millis();
      readBattery();
      batteryFilter();
    }

  // Get High-level descriptors (instrument data) - jab and shake for now
    instrument.updateInstrumentIMU(imu.getGyroX(), imu.getGyroY(), imu.getGyroZ());

  // send data via OSC
    if (settings.osc && WiFi.status() == WL_CONNECTED) {
      if (settings.mode==0) { // Send data via OSC ...
          // sending continuous data
            if (millis() - global.oscDelay > global.messageTimer) { 
              global.messageTimer = millis();
              sendContinuousOSC(settings.oscIP[0], settings.oscPORT[0]);
              sendContinuousOSC(settings.oscIP[1], settings.oscPORT[1]);
            }
          // send discrete (button/battery) data (only when it changes) or != 0
            if (global.lastCount != button.getCount()) {
              sendOSC(settings.oscIP[0], settings.oscPORT[0], "instrument/button/count", button.getCount());
              sendOSC(settings.oscIP[1], settings.oscPORT[1], "instrument/button/count", button.getCount());
              global.lastCount = button.getCount();
            } 
            if (global.lastTap != button.getTap()) {
              sendOSC(settings.oscIP[0], settings.oscPORT[0], "instrument/button/tap", button.getTap());
              sendOSC(settings.oscIP[1], settings.oscPORT[1], "instrument/button/tap",  button.getTap());
              global.lastTap = button.getTap();
            }
            if (global.lastDtap != button.getDTap()) {
              sendOSC(settings.oscIP[0], settings.oscPORT[0], "instrument/button/dtap",  button.getDTap());
              sendOSC(settings.oscIP[1], settings.oscPORT[1], "instrument/button/dtap",  button.getDTap());
              global.lastDtap = button.getDTap();
            }
            if (global.lastTtap != button.getTTap()) {
              sendOSC(settings.oscIP[0], settings.oscPORT[0], "instrument/button/ttap",  button.getTTap());
              sendOSC(settings.oscIP[1], settings.oscPORT[1], "instrument/button/ttap",  button.getTTap());
              global.lastTtap = button.getTTap();
            }
            if (global.lastFsr != fsr.getValue()) {
              sendOSC(settings.oscIP[0], settings.oscPORT[0], "raw/fsr", fsr.getValue());
              sendOSC(settings.oscIP[1], settings.oscPORT[1], "raw/fsr", fsr.getValue());
              sendOSC(settings.oscIP[0], settings.oscPORT[0], "norm/fsr", fsr.getNormValue());
              sendOSC(settings.oscIP[1], settings.oscPORT[1], "norm/fsr", fsr.getNormValue());
              global.lastFsr = fsr.getValue();
            }
            if (global.lastJab[0] != instrument.getJabX() || global.lastJab[1] != instrument.getJabY() || global.lastJab[2] != instrument.getJabZ()) {
              sendTrioOSC(settings.oscIP[0], settings.oscPORT[0], "instrument/jabxyz", instrument.getJabX(), instrument.getJabY(), instrument.getJabZ());
              sendTrioOSC(settings.oscIP[1], settings.oscPORT[1], "instrument/jabxyz", instrument.getJabX(), instrument.getJabY(), instrument.getJabZ());
              global.lastJab[0] = instrument.getJabX(); global.lastJab[1] = instrument.getJabY(); global.lastJab[2] = instrument.getJabZ();
            }
            if (global.lastShake[0] != instrument.getShakeX() || global.lastShake[1] != instrument.getShakeY() || global.lastShake[2] != instrument.getShakeZ()) {
              sendTrioOSC(settings.oscIP[0], settings.oscPORT[0], "instrument/shakexyz", instrument.getShakeX(), instrument.getShakeY(), instrument.getShakeZ());
              sendTrioOSC(settings.oscIP[1], settings.oscPORT[1], "instrument/shakexyz", instrument.getShakeX(), instrument.getShakeY(), instrument.getShakeZ());
              global.lastShake[0] = instrument.getShakeX(); global.lastShake[1] = instrument.getShakeY(); global.lastShake[2] = instrument.getShakeZ();
            }
            if (battery.lastPercentage != battery.percentage) {
              sendOSC(settings.oscIP[0], settings.oscPORT[0], "battery", battery.percentage);
              sendOSC(settings.oscIP[1], settings.oscPORT[1], "battery", battery.percentage);
              battery.lastPercentage = battery.percentage;
            }
      }
    }

  // receiving OSC
      //receiveOSC();

  // Check if setup mode has been called
    if (button.getHold()) {
      printf("\nLong button press, entering setup mode\n");
      settings.mode = 1;
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
      if (settings.mode == 1) { // 0:STA, 1:Setup(STA+AP+WebServer)
        ledcWrite(0, 255); // stays always on in setup mode
      } else { 
        if (WiFi.status() == WL_CONNECTED) { // blinks when connected, cycle when disconnected
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
      switch (settings.mode) { // 0:STA, 1:Setup(STA+AP+WebServer)
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
        case 1: // RGB: 255, 0, 255 (magenta)
            led.setInterval(1000);
            global.color = led.blink(255,20);
            tinypico.DotStar_SetPixelColor(global.color, 0, global.color);
          break;
        default: // if everything goes wrong it cycles through all colors (rainbow panic)
          tinypico.DotStar_CycleColor(25);
          break;
      }
    }
  #endif
} // end loop

//
// ┏━┓╻ ╻┏┓╻┏━╸╺┳╸╻┏━┓┏┓╻┏━┓
// ┣━ ┃ ┃┃┗┫┃   ┃ ┃┃ ┃┃┗┫┗━┓
// ╹  ┗━┛╹ ╹┗━╸ ╹ ╹┗━┛╹ ╹┗━┛
//

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
  if (settings.mode == 2) {Serial.println("Setup mode enabled");} 
  else {Serial.print("Data transmission mode: "); Serial.println(settings.mode);}
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
        settings.fsrOffset = doc["fsrOffset"];

        configFile.close();
        
        Serial.println("T-Stick module configuration file loaded.\n");
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
    doc["fsrOffset"] = settings.fsrOffset;
 
  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {Serial.println("Failed to open config file for writing!\n");}
  
  // Serialize JSON to file
    if (serializeJson(doc, configFile) == 0)
      Serial.println("Failed to write to file");
    else
      Serial.println("JSON file successfully saved!\n");
  
  configFile.close();
} //end save

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
    if(settings.mode == 0) {
      return "selected";
    } else {
      return "";
    }
  }
  if(var == "MODE1") {
    if(settings.mode == 1) {
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
  if(var == "GUITARAMIID") {
      char str[4];
      snprintf (str, sizeof(str), "%03d", settings.id);
      return str;
  }
  if(var == "GUITARAMIAUTH")
      return settings.author;
  if(var == "GUITARAMIINST")
      return settings.institution;
  if(var == "GUITARAMIVER")
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
  if(var == "GUITARAMIID") {
      char str[4];
      snprintf (str, sizeof(str), "%03d", settings.id);
      return str;
  }
  if(var == "GUITARAMIVER")
      return String(settings.firmware);
  if(var == "FSR")
      return String(settings.fsrOffset);

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
          settings.mode = 0;
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
            settings.mode = 0;
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
          if(request->hasParam("fsr", true)) {
            settings.fsrOffset = atoi(request->getParam("fsr", true)->value().c_str());
              printf("FSR offset (factory) received: %d\n", settings.fsrOffset);
          } else {
              printf("No FSR offset (factory) received\n");
          }
          request->send(SPIFFS, "/factory.html", String(), false, factoryProcessor);
          //request->send(200);
        }
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

    if (imu.dataAvailable()) {
        snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/%s/raw/accl",global.deviceName);
        OSCMessage msgAccl(namespaceBuffer);
          msgAccl.add(imu.getAccelX());
          msgAccl.add(imu.getAccelY());
          msgAccl.add(imu.getAccelZ());
          continuous.add(msgAccl);
          msgAccl.empty();
      
        snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/%s/raw/gyro",global.deviceName);
        OSCMessage msgGyro(namespaceBuffer);
          msgGyro.add(imu.getGyroX());
          msgGyro.add(imu.getGyroY());
          msgGyro.add(imu.getGyroZ());
          continuous.add(msgGyro);
          msgGyro.empty();
      
        snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/%s/raw/magn",global.deviceName);
        OSCMessage msgMagn(namespaceBuffer);
          msgMagn.add(imu.getMagX());
          msgMagn.add(imu.getMagY());
          msgMagn.add(imu.getMagZ());
          continuous.add(msgMagn);
          msgGyro.empty();

        // oscEndpoint.beginPacket(oscIP,port);
        // continuous.send(oscEndpoint);
        // oscEndpoint.endPacket();
        // continuous.empty();

        snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/%s/norm/accl",global.deviceName);
        OSCMessage msgnAccl(namespaceBuffer);
          msgnAccl.add(imu.getNormAccelX());
          msgnAccl.add(imu.getNormAccelY());
          msgnAccl.add(imu.getNormAccelZ());
          continuous.add(msgnAccl);
          msgnAccl.empty();
      
        snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/%s/norm/gyro",global.deviceName);
        OSCMessage msgnGyro(namespaceBuffer);
          msgnGyro.add(imu.getNormGyroX());
          msgnGyro.add(imu.getNormGyroY());
          msgnGyro.add(imu.getNormGyroZ());
          continuous.add(msgnGyro);
          msgnGyro.empty();

        snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/%s/norm/magn",global.deviceName);
        OSCMessage msgnMagn(namespaceBuffer);
          msgnMagn.add(imu.getNormMagX());
          msgnMagn.add(imu.getNormMagY());
          msgnMagn.add(imu.getNormMagZ());
          continuous.add(msgnMagn);
          msgnMagn.empty();

        // oscEndpoint.beginPacket(oscIP,port);
        // continuous.send(oscEndpoint);
        // oscEndpoint.endPacket();
        // continuous.empty(); 

        snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/%s/orientation",global.deviceName);
        OSCMessage msgQuat(namespaceBuffer);
          msgQuat.add(imu.getQuatI());
          msgQuat.add(imu.getQuatJ());
          msgQuat.add(imu.getQuatK());
          msgQuat.add(imu.getQuatReal());
          continuous.add(msgQuat);
          msgQuat.empty(); 

        snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/%s/instrument/ypr",global.deviceName);
        OSCMessage msgEuler(namespaceBuffer);
          msgEuler.add(imu.getYaw());
          msgEuler.add(imu.getPitch());
          msgEuler.add(imu.getRoll());
          continuous.add(msgEuler);
          msgEuler.empty(); 

        // oscEndpoint.beginPacket(oscIP,port);
        // continuous.send(oscEndpoint);
        // oscEndpoint.endPacket();
        // continuous.empty(); 

        snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/%s/raw/capsense",global.deviceName);
        OSCMessage msgtouchtouch(namespaceBuffer);
          #ifdef touch_TRILL
            for (int i=0;i<touch.touchSize;i++) {
              msgtouchtouch.add(touch.touch[i]);
            }
          #endif
          #ifdef touch_CAPSENSE
            for (int i=0;i<capsense.touchStripsSize;i++) {
              msgtouchtouch.add(capsense.data[i]);
            }
          #endif
          oscEndpoint.beginPacket(oscIP,port);
          msgtouchtouch.send(oscEndpoint);
          oscEndpoint.endPacket();
          msgtouchtouch.empty(); 

        snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/%s/instrument/touch/all",global.deviceName);
        OSCMessage msgtouchAll(namespaceBuffer);
          msgtouchAll.add(instrument_touch.touchAll);
          continuous.add(msgtouchAll);
          msgtouchAll.empty(); 

        snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/%s/instrument/touch/top",global.deviceName);
        OSCMessage msgtouchTop(namespaceBuffer);
          msgtouchTop.add(instrument_touch.touchTop);
          continuous.add(msgtouchTop);
          msgtouchTop.empty(); 

        snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/%s/instrument/touch/middle",global.deviceName);
        OSCMessage msgtouchMiddle(namespaceBuffer);
          msgtouchMiddle.add(instrument_touch.touchMiddle);
          continuous.add(msgtouchMiddle);
          msgtouchMiddle.empty(); 

        // oscEndpoint.beginPacket(oscIP,port);
        // continuous.send(oscEndpoint);
        // oscEndpoint.endPacket();
        // continuous.empty(); 

        snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/%s/instrument/touch/bottom",global.deviceName);
        OSCMessage msgtouchBottom(namespaceBuffer);
          msgtouchBottom.add(instrument_touch.touchBottom);
          continuous.add(msgtouchBottom);
          msgtouchBottom.empty(); 

        snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/%s/instrument/brush",global.deviceName);
        OSCMessage msgbrush(namespaceBuffer);
          msgbrush.add(instrument_touch.brush);
          continuous.add(msgbrush);
          msgbrush.empty(); 

        snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/%s/instrument/rub",global.deviceName);
        OSCMessage msgrub(namespaceBuffer);
          msgrub.add(instrument_touch.rub);
          continuous.add(msgrub);
          msgrub.empty(); 

        // oscEndpoint.beginPacket(oscIP,port);
        // continuous.send(oscEndpoint);
        // oscEndpoint.endPacket();
        // continuous.empty(); 

        snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/%s/instrument/multibrush",global.deviceName);
        OSCMessage msgmultiBrush(namespaceBuffer);
          for (int i=0;i<4;i++) {
            msgmultiBrush.add(instrument_touch.multiBrush[i]);
          }
          continuous.add(msgmultiBrush);
          msgmultiBrush.empty(); 

        snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/%s/instrument/multirub",global.deviceName);
        OSCMessage msgmultiRub(namespaceBuffer);
          for (int i=0;i<4;i++) {
            msgmultiRub.add(instrument_touch.multiRub[i]);
          }
          continuous.add(msgmultiRub);
          msgmultiRub.empty(); 

        snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/%s/button",global.deviceName);
        OSCMessage msgTouch(namespaceBuffer);
          msgTouch.add(button.getState());
          continuous.add(msgTouch);
          msgTouch.empty();

        oscEndpoint.beginPacket(oscIP,port);
        continuous.send(oscEndpoint);
        oscEndpoint.endPacket();
        continuous.empty(); 
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

void initLibmapper() {

  // Create device for libmapper
  if (WiFi.status() == WL_CONNECTED) {
      std::string lm_name = global.deviceName;
      dev = new mapper::Device(lm_name);

  // output = dev->add_signal(
  //              Direction::OUTGOING, 
  //              "my_output", // a unique name for the signal
  //              4,           // signal's vector length
  //              Type::INT32, // the signal's data type, one of Type::INT32, Type::FLOAT, or Type::DOUBLE
  //              "m/s",       // the signal's unit (optional). Use 0 (without quotes) if not specified
  //              min,         // the signal's minimum value (optional)
  //              max);        // the signal's maximum value (optional)
    
  int touchlmSize = touch.touchSize;
  std::string sigRawCapsense_name = "raw/capsense";
  lm.sigRawCapsense = dev->add_signal(mapper::Direction::OUTGOING, sigRawCapsense_name, touchlmSize, mapper::Type::INT32, 0, &lm.rawCapsenseMin, &lm.rawCapsenseMax);
  std::string sigRawGyroX_name = "raw/gyro/X";
  lm.sigRawGyroX = dev->add_signal(mapper::Direction::OUTGOING, sigRawGyroX_name, 1, mapper::Type::FLOAT, 0, &lm.rawGyroMin, &lm.rawGyroMax);
  std::string sigRawGyroY_name = "raw/gyro/Y";
  lm.sigRawGyroY = dev->add_signal(mapper::Direction::OUTGOING, sigRawGyroY_name, 1, mapper::Type::FLOAT, 0, &lm.rawGyroMin, &lm.rawGyroMax);
  std::string sigRawGyroZ_name = "raw/gyro/Z";
  lm.sigRawGyroZ = dev->add_signal(mapper::Direction::OUTGOING, sigRawGyroZ_name, 1, mapper::Type::FLOAT, 0, &lm.rawGyroMin, &lm.rawGyroMax);
  std::string sigRawAcclX_name = "raw/accl/X";
  lm.sigRawAcclX = dev->add_signal(mapper::Direction::OUTGOING, sigRawAcclX_name, 1, mapper::Type::INT32, 0, &lm.rawAcclMin, &lm.rawAcclMax);
  std::string sigRawAcclY_name = "raw/accl/Y";
  lm.sigRawAcclY = dev->add_signal(mapper::Direction::OUTGOING, sigRawAcclY_name, 1, mapper::Type::INT32, 0, &lm.rawAcclMin, &lm.rawAcclMax);
  std::string sigRawAcclZ_name = "raw/accl/Z";
  lm.sigRawAcclZ = dev->add_signal(mapper::Direction::OUTGOING, sigRawAcclZ_name, 1, mapper::Type::INT32, 0, &lm.rawAcclMin, &lm.rawAcclMax);
  std::string sigRawMagnX_name = "raw/magn/X";
  lm.sigRawMagnX = dev->add_signal(mapper::Direction::OUTGOING, sigRawMagnX_name, 1, mapper::Type::INT32, 0, &lm.rawMagnMin, &lm.rawMagnMax);
  std::string sigRawMagnY_name = "raw/magn/Y";
  lm.sigRawMagnY = dev->add_signal(mapper::Direction::OUTGOING, sigRawMagnY_name, 1, mapper::Type::INT32, 0, &lm.rawMagnMin, &lm.rawMagnMax);
  std::string sigRawMagnZ_name = "raw/magn/Z";
  lm.sigRawMagnZ = dev->add_signal(mapper::Direction::OUTGOING, sigRawMagnZ_name, 1, mapper::Type::INT32, 0, &lm.rawMagnMin, &lm.rawMagnMax);
  std::string sigRawFSR_name = "raw/fsr";
  lm.sigRawFSR = dev->add_signal(mapper::Direction::OUTGOING, sigRawFSR_name, 1, mapper::Type::INT32, 0, &lm.rawFSRMin, &lm.rawFSRMax);
  std::string sigYaw_name = "orientation/yaw";
  lm.sigYaw = dev->add_signal(mapper::Direction::OUTGOING, sigYaw_name, 1, mapper::Type::FLOAT, 0, &lm.yprMin, &lm.yprMax);
  std::string sigPitch_name = "orientation/pitch";
  lm.sigPitch = dev->add_signal(mapper::Direction::OUTGOING, sigPitch_name, 1, mapper::Type::FLOAT, 0, &lm.yprMin, &lm.yprMax);
  std::string sigRoll_name = "orientation/roll";
  lm.sigRoll = dev->add_signal(mapper::Direction::OUTGOING, sigRoll_name, 1, mapper::Type::FLOAT, 0, &lm.yprMin, &lm.yprMax);
  // std::string sigMagGyro_name = "gyro/magnitude";
  // lm.sigMagGyro = dev->add_signal(mapper::Direction::OUTGOING, sigMagGyro_name, 1, mapper::Type::FLOAT, 0, &lm.magMin, &lm.magMax);
  // std::string sigMagAccl_name = "accl/magnitude";
  // lm.sigMagAccl = dev->add_signal(mapper::Direction::OUTGOING, sigMagAccl_name, 1, mapper::Type::FLOAT, 0, &lm.magMin, &lm.magMax);
  // std::string sigMagMagn_name = "magn/magnitude";
  // lm.sigMagMagn = dev->add_signal(mapper::Direction::OUTGOING, sigMagMagn_name, 1, mapper::Type::FLOAT, 0, &lm.magMin, &lm.magMax);
  // std::string sigButton_name = "button/short";
  // lm.sigButton = dev->add_signal(mapper::Direction::OUTGOING, sigButton_name, 1, mapper::Type::INT32, 0, &lm.buttonMin, &lm.buttonMax);
  // std::string sigLongButton_name = "button/long";
  // lm.sigLongButton = dev->add_signal(mapper::Direction::OUTGOING, sigLongButton_name, 1, mapper::Type::INT32, 0, &lm.buttonMin, &lm.buttonMax);
  // std::string sigDoubleButton_name = "button/double";
  // lm.sigDoubleButton = dev->add_signal(mapper::Direction::OUTGOING, sigDoubleButton_name, 1, mapper::Type::INT32, 0, &lm.buttonMin, &lm.buttonMax);
  std::string sigtouchAll_name = "instrument/touchall";
  lm.sigtouchAll = dev->add_signal(mapper::Direction::OUTGOING, sigtouchAll_name, 1, mapper::Type::FLOAT, 0, &lm.instTouchMin, &lm.instTouchMax);
  std::string sigtouchTop_name = "instrument/touchtop";
  lm.sigtouchTop = dev->add_signal(mapper::Direction::OUTGOING, sigtouchTop_name, 1, mapper::Type::FLOAT, 0, &lm.instTouchMin, &lm.instTouchMax);
  std::string sigtouchMiddle_name = "instrument/touchmiddle";
  lm.sigtouchMiddle = dev->add_signal(mapper::Direction::OUTGOING, sigtouchMiddle_name, 1, mapper::Type::FLOAT, 0, &lm.instTouchMin, &lm.instTouchMax);
  std::string sigtouchBottom_name = "instrument/touchbottom";
  lm.sigtouchBottom = dev->add_signal(mapper::Direction::OUTGOING, sigtouchBottom_name, 1, mapper::Type::FLOAT, 0, &lm.instTouchMin, &lm.instTouchMax);
  std::string sigBrush_name = "instrument/brush";
  lm.sigBrush = dev->add_signal(mapper::Direction::OUTGOING, sigBrush_name, 1, mapper::Type::FLOAT, 0, &lm.genericMin, &lm.genericMax);
  std::string sigRub_name = "instrument/rub";
  lm.sigRub = dev->add_signal(mapper::Direction::OUTGOING, sigRub_name, 1, mapper::Type::FLOAT, 0, &lm.genericMin, &lm.genericMax);
  std::string sigMultiBrush_name = "instrument/multibrush";
  lm.sigMultiBrush = dev->add_signal(mapper::Direction::OUTGOING, sigMultiBrush_name, 4, mapper::Type::FLOAT, 0, &lm.genericMin, &lm.genericMax);
  std::string sigMultiRub_name = "instrument/multirub";
  lm.sigMultiRub = dev->add_signal(mapper::Direction::OUTGOING, sigMultiRub_name, 4, mapper::Type::FLOAT, 0, &lm.genericMin, &lm.genericMax);
  std::string sigShakeXYZ_name = "instrument/shakexyz";
  lm.sigShakeXYZ = dev->add_signal(mapper::Direction::OUTGOING, sigShakeXYZ_name, 3, mapper::Type::FLOAT, 0, &lm.genericMin, &lm.genericMax);
  std::string sigJabXYZ_name = "instrument/jabxyz";
  lm.sigJabXYZ = dev->add_signal(mapper::Direction::OUTGOING, sigJabXYZ_name, 3, mapper::Type::FLOAT, 0, &lm.genericMin, &lm.genericMax);
  }
}

void updateLibmapper() {
  dev->poll();

  std::vector<int> touchStripsVector(touch.touch, touch.touch + sizeof(touch.touch) / sizeof(int));
  std::vector<float> mBrushVector(instrument_touch.multiBrush, instrument_touch.multiBrush + sizeof(instrument_touch.multiBrush) / sizeof(float));
  std::vector<float> mRubVector(instrument_touch.multiRub, instrument_touch.multiRub + sizeof(instrument_touch.multiRub) / sizeof(float));
  std::vector<float> shakeVector(global.lastShake, global.lastShake + sizeof(global.lastShake) / sizeof(float));
  std::vector<float> jabVector(global.lastJab, global.lastJab + sizeof(global.lastJab) / sizeof(float));
  // TODO: incorporate vector into all firmware's logic (stop using int array)
  
  lm.sigRawCapsense.set_value(touchStripsVector);
  lm.sigRawFSR.set_value(fsr.getValue());
  lm.sigRawAcclX.set_value(imu.getAccelX());
  lm.sigRawAcclY.set_value(imu.getAccelY());
  lm.sigRawAcclZ.set_value(imu.getAccelZ());
  lm.sigRawGyroX.set_value(imu.getGyroX());
  lm.sigRawGyroY.set_value(imu.getGyroZ());
  lm.sigRawGyroZ.set_value(imu.getGyroZ());
  lm.sigRawMagnX.set_value(imu.getMagX());
  lm.sigRawMagnY.set_value(imu.getMagY());
  lm.sigRawMagnZ.set_value(imu.getMagZ());
  // lm.sigMagGyro.set_value(RawData.magGyro);
  // lm.sigMagAccl.set_value(RawData.magAccl);
  // lm.sigMagMagn.set_value(RawData.magMagn);
  // lm.sigButton.set_value(button.getTap());
  // lm.sigLongButton.set_value(button.getDTap());
  // lm.sigDoubleButton.set_value(button.getTTap());
  lm.sigYaw.set_value(imu.getYaw());
  lm.sigPitch.set_value(imu.getPitch());
  lm.sigRoll.set_value(imu.getRoll());
  lm.sigtouchAll.set_value(instrument_touch.touchAll);
  lm.sigtouchTop.set_value(instrument_touch.touchTop);
  lm.sigtouchMiddle.set_value(instrument_touch.touchMiddle);
  lm.sigtouchBottom.set_value(instrument_touch.touchBottom);
  lm.sigBrush.set_value(instrument_touch.brush);
  lm.sigRub.set_value(instrument_touch.rub);
  lm.sigMultiBrush.set_value(mBrushVector);
  lm.sigMultiRub.set_value(mRubVector);
  lm.sigShakeXYZ.set_value(shakeVector);
  lm.sigJabXYZ.set_value(jabVector);

  // mpr_sig_set_value(
  //     mpr_sig sig, 
  //     mpr_id inst, 
  //     int length,
  //     mpr_type type, 
  //     void *value
  //     );
  
  // mpr_sig_set_value(outputSignal, 0, 1, MPR_FLT, &&seqNumber);
  
//mpr_sig_set_value(lm.sigRawCapsense, 0, sizeof(RawData.touchStrips), MPR_INT32, &RawData.touchStrips);
//mpr_sig_set_value(lm.sigRawFSR, 0, 1, MPR_FLT, &RawData.fsr);
//mpr_sig_set_value(lm.sigRawPiezo, 0, 1, MPR_INT32, &RawData.piezo);
//mpr_sig_set_value(lm.sigRawAcclX, 0, 1, MPR_INT32, &RawData.accl[0]);
//mpr_sig_set_value(lm.sigRawAcclY, 0, 1, MPR_INT32, &RawData.accl[1]);
//mpr_sig_set_value(lm.sigRawAcclZ, 0, 1, MPR_INT32, &RawData.accl[2]);
//mpr_sig_set_value(lm.sigRawGyroX, 0, 1, MPR_FLT, &RawData.gyro[0]);
//mpr_sig_set_value(lm.sigRawGyroY, 0, 1, MPR_FLT, &RawData.gyro[1]);
//mpr_sig_set_value(lm.sigRawGyroZ, 0, 1, MPR_FLT, &RawData.gyro[2]);
//mpr_sig_set_value(lm.sigRawMagnX, 0, 1, MPR_INT32, &RawData.magn[0]);
//mpr_sig_set_value(lm.sigRawMagnY, 0, 1, MPR_INT32, &RawData.magn[1]);
//mpr_sig_set_value(lm.sigRawMagnZ, 0, 1, MPR_INT32, &RawData.magn[2]);
//mpr_sig_set_value(lm.sigOrientationQ1, 0, 1, MPR_FLT, &RawData.quat[0]);
//mpr_sig_set_value(lm.sigOrientationQ2, 0, 1, MPR_FLT, &RawData.quat[1]);
//mpr_sig_set_value(lm.sigOrientationQ3, 0, 1, MPR_FLT, &RawData.quat[2]);
//mpr_sig_set_value(lm.sigOrientationQ4, 0, 1, MPR_FLT, &RawData.quat[3]);
//mpr_sig_set_value(lm.sigMagGyro, 0, 1, MPR_FLT, &RawData.magGyro);
//mpr_sig_set_value(lm.sigMagAccl, 0, 1, MPR_FLT, &RawData.magAccl);
//mpr_sig_set_value(lm.sigMagMagn, 0, 1, MPR_FLT, &RawData.magMagn);
//mpr_sig_set_value(lm.sigButton, 0, 1, MPR_INT32, &RawData.buttonShort);
//mpr_sig_set_value(lm.sigLongButton, 0, 1, MPR_INT32, &RawData.buttonLong);
//mpr_sig_set_value(lm.sigDoubleButton, 0, 1, MPR_INT32, &RawData.buttonDouble);
//mpr_sig_set_value(lm.sigYaw, 0, 1, MPR_FLT, &InstrumentData.ypr[0]);
//mpr_sig_set_value(lm.sigPitch, 0, 1, MPR_FLT, &InstrumentData.ypr[1]);
//mpr_sig_set_value(lm.sigRoll, 0, 1, MPR_FLT, &InstrumentData.ypr[2]);
//mpr_sig_set_value(lm.sigtouchAll, 0, 1, MPR_FLT, &InstrumentData.touchAll);
//mpr_sig_set_value(lm.sigtouchTop, 0, 1, MPR_FLT, &InstrumentData.touchTop);
//mpr_sig_set_value(lm.sigtouchMiddle, 0, 1, MPR_FLT, &InstrumentData.touchMiddle);
//mpr_sig_set_value(lm.sigtouchBottom, 0, 1, MPR_FLT, &InstrumentData.touchBottom);
//mpr_sig_set_value(lm.sigBrush, 0, 1, MPR_FLT, &InstrumentData.brush);
//mpr_sig_set_value(lm.sigRub, 0, 1, MPR_FLT, &InstrumentData.rub);
//mpr_sig_set_value(lm.sigMultiBrush, 0, sizeof(InstrumentData.multiBrush)/sizeof(InstrumentData.multiBrush[0]), MPR_FLT, &RawData.touchStrips);
//mpr_sig_set_value(lm.sigMultiRub, 0, sizeof(InstrumentData.multiRub)/sizeof(InstrumentData.multiRub[0]), MPR_FLT, &RawData.touchStrips);
//mpr_sig_set_value(lm.sigShakeXYZ, 0, sizeof(InstrumentData.shakeXYZ)/sizeof(InstrumentData.shakeXYZ[0]), MPR_FLT, &RawData.touchStrips);
//mpr_sig_set_value(lm.sigJabXYZ, 0, sizeof(InstrumentData.jabXYZ)/sizeof(InstrumentData.jabXYZ[0]), MPR_FLT, &RawData.touchStrips);
}