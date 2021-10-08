//********************************************************************************//
//  Sopranino T-Stick 2GW - Wemos D1 - USB -WiFi                                  //
//  Input Devices and Music Interaction Laboratory                                //
//  Created: February 2018 by Alex Nieva                                          //
//  Notes   : Based on test program for reading CY8C201xx using I2C               //
//            by Joseph Malloch 2011                                              //
//                                                                                //
//            Adapted to work with Arduino IDE 1.8.5 and T-Stick Sopranino 2GW    //
//********************************************************************************//
// See the CY8C201xx Register Reference Guide for more info:
// http://www.cypress.com/?docID=24620
// This chip is outdated now, see newer designs.


//############ Set board and config options here##############
//#define _ESP32
#define _ESP8266
//#define _SMARTCONFIG
//###########################################################


#ifdef _ESP32
#include "WiFi.h"
#include <HardwareSerial.h>

HardwareSerial Serial1(1);

#define SERIAL1_RXPIN 32
#define SERIAL1_TXPIN 33
#endif

#ifdef _ESP8266
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#endif

// include Wire library to read and write I2C commands:
#include <Wire.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>

// EEPROM library
#include <EEPROM.h>

// The SFE_LSM9DS0 requires both the SPI and Wire libraries.
// Unfortunately, you'll need to include both in the Arduino
// sketch, before including the SFE_LSM9DS0 library.
#include <SPI.h> // Included for SFE_LSM9DS0 library
#include <SFE_LSM9DS0.h>

// Library for MQTT
//#include <PubSubClient.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#define DEBUG true
#define CALIB false

//////////////////////
// WiFi Definitions //
//////////////////////
// WiFi Network variables
const char *ssid = "SopraninoWiFi-181";           //Ap SSID
const char *password = "password";                //Ap Password
String Essid = "";                 //EEPROM Network SSID
String Epass = "";                 //EEPROM Network Password
String sssid = "";                 //Read SSID From Web Page
String passs = "";                 //Read Password From Web Page
String standalone = "";            //Standalone option for Sopranino
String oscIP = "";
String oscPORT = "";

ESP8266WebServer server(80); // Changed WiFiServer to ESP8266WebServer for http services. Johnty's implementation uses the former.
bool wifiConnected = false;
bool externalNetwork = false;
bool networkChange = false;
int timeout1 = 5000; bool timeout1check = false;
WiFiUDP oscEndpoint;            // A UDP instance to let us send and receive packets over UDP
IPAddress oscEndpointIP;        // remote IP of your computer
int oscEndpointPORT;            // remote port to receive OSC
const unsigned int portLocal = 8888;       // local port to listen for OSC packets (actually not used for sending)
bool udpConnected = false;
bool sendOSC = true;
//char incomingPacket[255];  // buffer for incoming packets
static int bufferFromHost[4] = {0, 0, 0, 0};


///////////////////////
//  IMU Definitions  //
///////////////////////
// Comment out this section if you're using SPI
// SDO_XM and SDO_G are both grounded, so our addresses are:
#define LSM9DS0_XM  0x1D // Would be 0x1E if SDO_XM is LOW
#define LSM9DS0_G   0x6B // Would be 0x6A if SDO_G is LOW
// Create an instance of the LSM9DS0 library called `dof` the
// parameters for this constructor are:
// [SPI or I2C Mode declaration],[gyro I2C address],[xm I2C add.]
LSM9DS0 dof(MODE_I2C, LSM9DS0_G, LSM9DS0_XM);

// global constants for 9 DoF fusion and AHRS (Attitude and Heading Reference System)
#define GyroMeasError PI * (40.0f / 180.0f)       // gyroscope measurement error in rads/s (shown as 3 deg/s)
#define GyroMeasDrift PI * (0.0f / 180.0f)      // gyroscope measurement drift in rad/s/s (shown as 0.0 deg/s/s)
// There is a tradeoff in the beta parameter between accuracy and response speed.
// In the original Madgwick study, beta of 0.041 (corresponding to GyroMeasError of 2.7 degrees/s) was found to give optimal accuracy.
// However, with this value, the LSM9SD0 response time is about 10 seconds to a stable initial quaternion.
// Subsequent changes also require a longish lag time to a stable output, not fast enough for a quadcopter or robot car!
// By increasing beta (GyroMeasError) by about a factor of fifteen, the response time constant is reduced to ~2 sec
// I haven't noticed any reduction in solution accuracy. This is essentially the I coefficient in a PID control sense; 
// the bigger the feedback coefficient, the faster the solution converges, usually at the expense of accuracy. 
// In any case, this is the free parameter in the Madgwick filtering and fusion scheme.
#define beta sqrt(3.0f / 4.0f) * GyroMeasError   // compute beta
#define zeta sqrt(3.0f / 4.0f) * GyroMeasDrift   // compute zeta, the other free parameter in the Madgwick scheme usually set to a small or zero value

// IMU buffers and intervals
float rawAccel[3] = {0, 0, 0};
float rawGyro[3] = {0, 0, 0};
float rawMag[3] = {0, 0, 0};
//Rotated variables
float outAccel[3] = {0, 0, 0};
float outGyro[3] = {0, 0, 0};
float outMag[3] = {0, 0, 0};

//float pitch, yaw, roll, heading;
float abias[3] = {0, 0, 0}, gbias[3] = {0, 0, 0};
float q[4] = {1.0f, 0.0f, 0.0f, 0.0f};    // vector to hold quaternion
float q1[4] = {0.0f, 0.0f, 0.0f, 0.0f}; // vector to tare the systems of coordinate
float R[3][3] = {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}};
float temperature;
bool IMUcalibrated = false;
bool rotateManually = true;

float deltat = 0.0f;        // integration interval for both filter schemes
uint32_t lastUpdateQuat = 0;    // used to calculate integration interval
uint32_t NowQuat = 0;           // used to calculate integration interval

uint32_t AccelDeltaRead = 5000; //microseconds
uint32_t PreviousAccelRead = 0;
uint32_t GyroDeltaRead = 5263; //microseconds
uint32_t PreviousGyroRead = 0;
uint32_t MagDeltaRead = 80000; //microseconds
uint32_t PreviousMagRead = 0;
uint32_t dataTransferRate = 50; // sending data at 20Hz
uint32_t deltaTransferRate = 0;
unsigned int pressure = 0;

// ESP8266 functions
//void initHardware(void);
boolean connectUDP(void);
boolean connectWifi(void);

// defaults
unsigned int infoTstick[2] = {0, 0};    // serial number and firmware revision
int xres = D5;  // XRES pin on one of the CY8C201xx chips is connected to Arduino pin 13
int xres1 = D0;
int pressurePin = A0;
int ledPin = D3;
int ledStatus = 0;
int ledTimer = 1000;
byte touch1 = 0;
byte touch[2] = {0, 0};
byte touchMask[2] = {255, 255};
unsigned int calibrationData[2] = {0, 1023};

// the current address in the EEPROM (i.e. which byte
// we're going to write to next)
int addr = 0;             // Start of instrument info
int addrNet = addr + 12;  // Start of Network info

//control definitions
unsigned long then = 0;
unsigned long now = 0;
unsigned long started = 0;
byte calibrate = 0;

// I2C adresses CY8C201xx
#define I2C_ADDR0 0x00
#define I2C_ADDR0_NOPULL 0x80
#define I2C_ADDR1 0x02
#define I2C_ADDR1_NOPULL 0x82

// some CY8C201xx registers
#define INPUT_PORT0 0x00
#define INPUT_PORT1 0x01
#define CS_ENABLE0 0x06
#define CS_ENABLE1 0x07
#define I2C_DEV_LOCK 0x79
#define I2C_ADDR_DM 0x7C
#define COMMAND_REG 0xA0

// Secret codes for locking/unlocking the I2C_DEV_LOCK register
byte I2CDL_KEY_UNLOCK[3] = {0x3C, 0xA5, 0x69};
byte I2CDL_KEY_LOCK[3] = {0x96, 0x5A, 0xC3};

unsigned long lastRead=0;
int interval = 15;

void setup() {

  // start serial interface
  oscEndpointIP.fromString("192.168.1.118");
  oscEndpointPORT = 8000;
  
  Serial.begin(115200);

  //Configure I2C bus
  setupI2C();
  //IMU calibration
//  Serial.println();
//  Serial.println("Calibrating IMU... ");
//  while (!calibIMU()) {
//    Serial.print(".");
//  }
//  Serial.println();
//  Serial.println("Calibration Done");

  EEPROM.begin(512);
  Serial.println();

  readSettings();
  
  pinMode(ledPin, OUTPUT);

  if (setupWiFi()) {
    Serial.println("SetupWiFi() was successful");
  }
  else {
    Serial.println("SetupWiFi() was unsuccessful");
  }

  Serial.println();
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());

  server.on("/",D_AP_SER_Page); 
  server.on("/a",Get_Req);                //If submit button is pressed get the new SSID and Password and store it in EEPROM 
  server.on("/b",STalone);
  server.on("/c",setOSCendpoint);
  server.on("/d",writeNetworkToEEPROM);
  Serial.println("Starting webserver");
  server.begin();
  delay(100);

  udpConnected = connectUDP();
  delay(100);

//  if (CALIB) calib();
}

void loop() {
  // put your main code here, to run repeatedly:
  server.handleClient();
  if (externalNetwork == true & networkChange == true){
    WiFi.mode(WIFI_STA);
    WiFi.begin(sssid.c_str(),passs.c_str());
    // Wait for connection
    Serial.print("Connecting to requested network");
    int then = millis();
    while ( WiFi.status() != WL_CONNECTED && timeout1check == false) {
      delay ( 500 );
      Serial.print ( "." );
      if (millis() - then > timeout1) {
        timeout1check = true;
        Serial.println("Connection to requested network failed... Creating Access Point.");
        createAP();
      }
    }
    if (timeout1check == false) {
      Serial.println ( "" );
      Serial.print ( "Connected to " ); Serial.println (WiFi.SSID());
      Serial.print ( "IP address: " ); Serial.println ( WiFi.localIP() );
      networkChange = false;
    }
  } // end of network change handling
  

  byte dataRec = OSCMsgReceive();

  if (dataRec){ //Check for OSC messages from host computers
    if (DEBUG){
      Serial.println();
      for (int i = 0; i<4; i++){
      Serial.printf("From Max %d: ", i); Serial.println(bufferFromHost[i]);
      }  
      Serial.println();
    }
    char message = bufferFromHost[0];
    OSCMessage msg0("/information");

    switch (message){
      case 's': // start message,
        msg0.add(infoTstick[0]);
        msg0.add(infoTstick[1]);
        oscEndpoint.beginPacket(oscEndpointIP, oscEndpointPORT);
        msg0.send(oscEndpoint);
        oscEndpoint.endPacket();
        msg0.empty();
        
        started = millis();
        break;
      case 'x': // stop message,
        started = 0;
        break;
      case 'c': // calibrate message
        switch(bufferFromHost[1]){
          case 1: // FSR calibration
            calibrationData[0] = 1023;
            calibrationData[1] = 0;
            calibrate = 1;
            bufferFromHost[1] = 0;            
            break;
          default:
            calibrate = 0;
            break;
        }
        break;
      case 'w':     //write settings
        switch((char)bufferFromHost[1]){
          case 'i': //write info
            infoTstick[0] = bufferFromHost[2];
            infoTstick[1] = bufferFromHost[3];
            bufferFromHost[1] = 0;            
            bufferFromHost[2] = 0;
            bufferFromHost[3] = 0;
            break;            
          case 'T': //write touch mask
            touchMask[0] = bufferFromHost[2];
            touchMask[1] = bufferFromHost[3];
            bufferFromHost[1] = 0;            
            bufferFromHost[2] = 0;
            bufferFromHost[3] = 0;
            break;
          case 'w':
            writeSettings();
            bufferFromHost[1] = 0;            
            break;
          case 'r': 
            readSettings();
            bufferFromHost[1] = 0;            
            break;
          default:
            break;            
        }
      default:
        break;
    }
  }

  now = millis();

  if (now < started + 2000){ // needs a ping/keep-alive every 2 seconds or less or will time-out
    if (sendOSC){
      if (millis()-lastRead>interval)
      {
        lastRead = millis();
        
      // get the touch values from 2 x CY8C201xx chips  
          OSCMessage msg1("/rawcapsense");
          msg1.add((int)readTouch(I2C_ADDR0) & touchMask[0]);
          msg1.add((int)readTouch(I2C_ADDR1) & touchMask[1]);
          //msg1.add((int)touch[0]);
          //msg1.add((int)touch[1]);
          oscEndpoint.beginPacket(oscEndpointIP, oscEndpointPORT);
          msg1.send(oscEndpoint);
          oscEndpoint.endPacket();
          msg1.empty();
      }
    
      if ((micros() - PreviousGyroRead) > GyroDeltaRead){
        printGyro();  // Print "G: gx, gy, gz"
        PreviousGyroRead = micros();
      }
      if ((micros() - PreviousAccelRead) > AccelDeltaRead){
        printAccel(); // Print "A: ax, ay, az"
        PreviousAccelRead = micros();
      }
      if ((micros() - PreviousMagRead) > MagDeltaRead){
        printMag();   // Print "M: mx, my, mz"
        PreviousMagRead = micros();   
      }

//      // quaternion update and coordinate rotation
//      NowQuat = micros();
//      deltat = ((NowQuat - lastUpdateQuat)/1000000.0f); // set integration time by time elapsed since last filter update
//      lastUpdateQuat = NowQuat;
//      MadgwickQuaternionUpdate(outAccel[0], outAccel[1], outAccel[2], outGyro[0]*PI/180.0f, outGyro[1]*PI/180.0f, outGyro[2]*PI/180.0f, outMag[0], outMag[1], outMag[2]);

      if ((millis() - deltaTransferRate) > dataTransferRate){
        OSCMessage msg2("/rawgyro");
        msg2.add(outGyro[0]);
        msg2.add(outGyro[1]);
        msg2.add(outGyro[2]);
        oscEndpoint.beginPacket(oscEndpointIP, oscEndpointPORT);
        msg2.send(oscEndpoint);
        oscEndpoint.endPacket();
        msg2.empty();
    
        OSCMessage msg3("/rawaccel");
        msg3.add(outAccel[0]);
        msg3.add(outAccel[1]);
        msg3.add(outAccel[2]);
        oscEndpoint.beginPacket(oscEndpointIP, oscEndpointPORT);
        msg3.send(oscEndpoint);
        oscEndpoint.endPacket();
        msg3.empty();
    
        OSCMessage msg4("/rawmag");
        msg4.add(outMag[0]);
        msg4.add(outMag[1]);
        msg4.add(outMag[2]);
        oscEndpoint.beginPacket(oscEndpointIP, oscEndpointPORT);
        msg4.send(oscEndpoint);
        oscEndpoint.endPacket();
        msg4.empty();
    
        pressure = analogRead(pressurePin);
        if (calibrate == 1) {
          calibrationData[0] = constrain(min(calibrationData[0], pressure), 0, 1023);
          calibrationData[1] = constrain(max(calibrationData[1], pressure), 0, 1023);
        }
        pressure = constrain(map(pressure, calibrationData[0], calibrationData[1], 0, 1023), 0, 1023);
        OSCMessage msg5("/rawpressure");
        msg5.add(pressure);
        oscEndpoint.beginPacket(oscEndpointIP, oscEndpointPORT);
        msg5.send(oscEndpoint);
        oscEndpoint.endPacket();
        msg5.empty();
        deltaTransferRate = millis();
      }
    }    
    ledBlink();
    then = now;
  }
  else {
    now = millis();
    if ((now - then) > ledTimer){
      ledBlink();
      then = now;
    }
    else if ( then > now)
      then = 0;
  }
}

void ledBlink() {
  ledStatus = (ledStatus + 1) % 2;
  digitalWrite(ledPin, ledStatus);
}

byte readTouch(int address) {
  byte touch = 0;
  // request Register 00h: INPUT_PORT0
  Wire.beginTransmission(address);
  Wire.write(INPUT_PORT0);
  Wire.endTransmission();
  
  Wire.requestFrom(address, 1);
  while (Wire.available()) {
  touch = Wire.read() << 4;
  }
  // request Register 01h: INPUT_PORT1
  Wire.beginTransmission(address);
  Wire.write(INPUT_PORT1);
  Wire.endTransmission();
  
  Wire.requestFrom(address, 1);
  while (Wire.available()) {
  touch |= Wire.read();
  }

  byte touch_temp = 0;

  for (int i=0; i<8; i++){
    byte mult = 1<<i;
    byte temp = ((touch & mult)>>i)%2;
    switch (i) {
      case 0:
        touch_temp |= temp << 7;
        break;
      case 1:
        touch_temp |= temp << 4;
        break;
      case 2:
        touch_temp |= temp << 5;
        break;
      case 3:
        touch_temp |= temp << 2;
        break;
      case 4:
        touch_temp |= temp << 1;
        break;
      case 5:
        touch_temp |= temp << 6;
        break;
      case 6:
        touch_temp |= temp << 3;
        break;
      case 7:
        touch_temp |= temp << 0;
        break;
    }
  }

  touch = touch_temp;
  
  return touch;

}

void D_AP_SER_Page() {
  int Tnetwork=0,i=0,len=0,p=0;
  String st="",s="";                                    //String array to store the SSID's of available networks
  Tnetwork = WiFi.scanNetworks();            //Scan for total networks available
  st = "<ul>";
  for (int i = 0; i < Tnetwork; ++i)
    {
      // Print SSID and RSSI for each network found
      if (WiFi.RSSI(i) > -80) {
      st += "<li>";
      p   = ++p;
      st += p;
      st += ": ";
      st += WiFi.SSID(i);
      st += " (";
      st += WiFi.RSSI(i);
      st += ")";
      st += (WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*";
      st += "</li>";
      }
    }
      st += "</ul>";
    IPAddress ip = WiFi.softAPIP();                  //Get ESP8266 IP Adress
    //String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
    s = "\n\r\n<!DOCTYPE HTML>\r\n<html><h1>SopraninoWiFi-181 Configuration Webpage</h1> ";
    //s += ipStr;
    s += "<p>";
    s += st;
    s += "<form method='get' action='a'><label>SSID: </label><input name='ssid' length=32><label>Password: </label><input name='pass' length=64><input type='submit'></form></p>";
    s += "<p><form method='get' action='b'><label>Switch to Standalone mode (yes/no)</label><input name='standalone' length=5><input type='submit'></form></p>";
    s += "<p><form method='get' action='d'><label>Save network to EEPROM</label><input type='submit'></form></p>";
    s += "<hr>";
    s += "<h2>OSC target configuration</h2>";
    s += "<form method='get' action='c'><label>Dest IP: </label><input name='oscIP' value=192.168.1.18><br>";
    s += "<label>Port: </label><input name='oscPORT' value='8000'><br>";
    s += "<input type='submit' value='Set OSC Destination'></form>";
    s += "</html>\r\n\r\n";
      
    server.send( 200 , "text/html", s);
}


void Get_Req(){
  if (server.hasArg("ssid") && server.hasArg("pass")){  
   sssid=server.arg("ssid");//Get SSID
   passs=server.arg("pass");//Get Password
   Serial.print("External WiFi Network: "); Serial.println(sssid);
   externalNetwork = true; networkChange = true;
//  }

//  if(sssid.length()>1 && passs.length()>1){
//    ClearEeprom();//First Clear Eeprom
//    delay(10);
//    for (int i = 0; i < sssid.length(); ++i)
//          {
//            EEPROM.write(i, sssid[i]);
//          }
//        
//    for (int i = 0; i < passs.length(); ++i)
//          {
//            EEPROM.write(32+i, passs[i]);
//          }    
//    EEPROM.commit();
        
    String s = "\r\n\r\n<!DOCTYPE HTML>\r\n<html><h1>SopraninoWiFi-181 Web Server</h1> ";
    s += "<p>Connected to selected WiFi network.</html>\r\n\r\n";
    server.send(200,"text/html",s);
//   }
  }   
}

void STalone() {
  standalone = server.arg("standalone");
  if (standalone == "yes") {
    Serial.println("Setting Sopranino in Standalone mode");
    createAP();
  }
}

void setOSCendpoint() {
  if (server.hasArg("oscIP") && server.hasArg("oscPORT")) {
    oscEndpointIP.fromString(server.arg("oscIP"));
    oscEndpointPORT = server.arg("oscPORT").toInt();
    Serial.print("New oscEndpointIP: "); Serial.println(oscEndpointIP);
    Serial.print("New oscEndpointPORT: "); Serial.println(oscEndpointPORT);
  }

  String s = "\r\n\r\n<!DOCTYPE HTML>\r\n<html><h1>SopraninoWiFi-181 Configuration Webpage</h1> ";
  s += "<p>OSC endpoint IP and port set.</p>";
//  s += "<button type='submit' onclick='/'>Return home</button></html>\r\n\r\n";
  s += "<br><a href='/'>Return home</a></html>\r\n\r\n";
  server.send(200,"text/html",s);
}

void readSettings(){
  Serial.println("Reading settings from EEPROM...");
//  Serial.print("byte 0: "); Serial.println(EEPROM.read(addr));
//  Serial.print("byte 1: "); Serial.println(EEPROM.read(addr+1));
//  Serial.print("byte 2: "); Serial.println(EEPROM.read(addr+2));
//  Serial.print("byte 3: "); Serial.println(EEPROM.read(addr+3));
  infoTstick[0] = (unsigned int)(EEPROM.read(addr+1)) + (unsigned int)(EEPROM.read(addr)<<8);
  infoTstick[1] = (unsigned int)(EEPROM.read(addr+3)) + (unsigned int)(EEPROM.read(addr+2)<<8);
  if (DEBUG)
  {
    Serial.print("value infoTstick1: "); Serial.println(infoTstick[0]);
    Serial.print("value infoTstick2: "); Serial.println(infoTstick[1]);
  }
  calibrationData[0] = (unsigned int)(EEPROM.read(addr+5)) + (unsigned int)(EEPROM.read(addr+4)<<8);
  calibrationData[1] = (unsigned int)(EEPROM.read(addr+7)) + (unsigned int)(EEPROM.read(addr+6)<<8);  
}

void writeSettings(){
  Serial.println("Writing settings to EEPROM...");
  EEPROM.write(addr,   (byte)(infoTstick[0]/256));
//  Serial.print("byte 0: "); Serial.println((byte)(infoTstick[0]/256));
  EEPROM.write(addr+1, (byte)(infoTstick[0]%256));
//  Serial.print("byte 1: "); Serial.println((byte)(infoTstick[0]%256));
  EEPROM.write(addr+2, (byte)(infoTstick[1]/256));
//  Serial.print("byte 2: "); Serial.println((byte)(infoTstick[1]/256));
  EEPROM.write(addr+3, (byte)(infoTstick[1]%256));
//  Serial.print("byte 3: "); Serial.println((byte)(infoTstick[1]%256));
  EEPROM.write(addr+4, (byte)(calibrationData[0]/256));
  EEPROM.write(addr+5, (byte)(calibrationData[0]%256));
  EEPROM.write(addr+6, (byte)(calibrationData[1]/256));
  EEPROM.write(addr+7, (byte)(calibrationData[1]%256));
  
  EEPROM.commit();

}

