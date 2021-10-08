#include <Wire.h>
#include <SPI.h>
#include <Adafruit_LSM9DS1.h>
#include <Adafruit_Sensor.h>  // not used in this demo but required!

// i2c
Adafruit_LSM9DS1 lsm = Adafruit_LSM9DS1();

#define LSM9DS1_SCK A5
#define LSM9DS1_MISO 12
#define LSM9DS1_MOSI A4
#define LSM9DS1_XGCS 6
#define LSM9DS1_MCS 5
// You can also use software SPI
//Adafruit_LSM9DS1 lsm = Adafruit_LSM9DS1(LSM9DS1_SCK, LSM9DS1_MISO, LSM9DS1_MOSI, LSM9DS1_XGCS, LSM9DS1_MCS);
// Or hardware SPI! In this case, only CS pins are passed in
//Adafruit_LSM9DS1 lsm = Adafruit_LSM9DS1(LSM9DS1_XGCS, LSM9DS1_MCS);

bool Capsense_OK = false;
bool IMU_OK = false;

//////////////////////////
// Capsense Definitions //
//////////////////////////


#define SENSOR_EN 0x00
#define FSS_EN 0x02
#define SENSITIVITY0 0x08
#define SENSITIVITY1 0x09
#define SENSITIVITY2 0x0A
#define SENSITIVITY3 0x0B
#define DEVICE_ID 0x90    // Should return 0xA05 (returns 2 bytes)
#define FAMILY_ID 0x8F //143
#define SYSTEM_STATUS 0x8A
#define I2C_ADDR 0x37     // Should return 0x37
#define REFRESH_CTRL 0x52
#define SENSOR_EN 0x00    // We should set it to 0xFF for 16 sensors
#define BUTTON_STAT 0xAA  // Here we red the status of the sensors (2 bytes)
#define CTRL_CMD 0x86     // To configure the Capsense
#define CTRL_CMD_STATUS 0x88
#define CTRL_CMD_ERROR 0x89
#define BUTTON_LBR 0x1F
#define SPO_CFG 0x4C      //CS15 configuration address
#define GPO_CFG 0x40
#define CALC_CRC 0x94
#define CONFIG_CRC 0x7E

byte touch[2] = {0, 0};

#define DEBUG 1

void initCapsense() {
  Wire.setClock(400000);

  if (DEBUG) {
    ///////
    byte infoboard = 0;
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(SYSTEM_STATUS);
    Wire.endTransmission();

    // This should be 0 since a configuration other than the factory is loaded.
    Wire.requestFrom(I2C_ADDR, 1);
    Serial.print("Wire.available First: ");
    Serial.println(Wire.available());
    infoboard = Wire.read();
    Serial.println("This should be 0");
    Serial.print("SYSTEM_STATUS: ");
    Serial.println(infoboard);
    Wire.endTransmission();

    ///////
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(FAMILY_ID);
    Wire.endTransmission();

    Wire.requestFrom(I2C_ADDR, 1);
    Serial.print("Wire.available SECOND: ");
    Serial.println(Wire.available());
    infoboard = Wire.read();
    Serial.println("This should be 154");
    Serial.print("FAMILY_ID: ");
    Serial.println(infoboard);
    Wire.endTransmission();


    //  Wire.beginTransmission(I2C_ADDR);
    //  Wire.write(DEVICE_ID);
    //  Wire.endTransmission();
    //  //delay(100);
    //
    //  Wire.requestFrom(I2C_ADDR,2);
    //  Serial.print("Wire.available THIRD: ");
    //  Serial.println(Wire.available());
    //  while (Wire.available()) {
    //    byte c = Wire.read();
    //    Serial.println(c);
    //  }
    //delay(100);
    //  Wire.beginTransmission(I2C_ADDR);
    //  Wire.write(REFRESH_CTRL);
    //  Wire.endTransmission();
    //  //delay(100);
    //
    //  Wire.requestFrom(I2C_ADDR,1);
    //  Serial.print("Wire.available sixth: ");
    //  Serial.println(Wire.available());
    //  infoboard = Wire.read();
    //  Serial.print("REFRESH_CTRL VALUE: ");
    //  Serial.println(infoboard);
    //  Wire.endTransmission();
    //delay(100);
  }

  //**********************************
  // Capsense pins configuration
  //
  // Array to enable each of the 16 capsense sensors
  byte sensors[2] = {0xFFu, 0xFFu};

  // Originally the come as 0x00, 0xFF
  Wire.beginTransmission(I2C_ADDR);
  Wire.write(SENSOR_EN);
  Wire.write(sensors, 2);
  Wire.endTransmission();

  // Not actually necessary I think.
  Wire.beginTransmission(I2C_ADDR);
  Wire.write(FSS_EN);
  //Wire.write(sensors,2);
  Wire.write(0x00);
  Wire.write(0x00);
  Wire.endTransmission();

  //  // Sensitivity
  //  Wire.beginTransmission(I2C_ADDR);
  //  Wire.write(SENSITIVITY0);
  //  Wire.write(B10101010);
  //  Wire.endTransmission();
  //
  //  Wire.beginTransmission(I2C_ADDR);
  //  Wire.write(SENSITIVITY1);
  //  Wire.write(B10101010);
  //  Wire.endTransmission();
  //
  //  Wire.beginTransmission(I2C_ADDR);
  //  Wire.write(SENSITIVITY2);
  //  Wire.write(B10101010);
  //  Wire.endTransmission();
  //
  //  Wire.beginTransmission(I2C_ADDR);
  //  Wire.write(SENSITIVITY3);
  //  Wire.write(B10101010);
  //  Wire.endTransmission();

  // Special Purpose Output pin configuration
  Wire.beginTransmission(I2C_ADDR);
  Wire.write(SPO_CFG);
  Wire.write(B00010101);
  Wire.endTransmission();
  //
  //  Wire.beginTransmission(I2C_ADDR);
  //  Wire.write(SPO_CFG);
  //  Wire.endTransmission();
  //  //delay(100);
  //
  //  Wire.requestFrom(I2C_ADDR,1);
  //  Serial.print("Wire.available SPO_CFG: ");
  //  Serial.println(Wire.available());
  //  infoboard = Wire.read();
  //  Serial.print("SPO_CFG: ");
  //  Serial.println(infoboard,BIN);
  //  Wire.endTransmission();

  // Let's read again the SENSOR_EN just to confirm that nothing has been written yet. To be Deleted
  Wire.beginTransmission(I2C_ADDR);
  Wire.write(SENSOR_EN);
  Wire.endTransmission();

  Wire.requestFrom(I2C_ADDR, 2);
  Serial.print("Wire.available FOURTH: ");
  Serial.println(Wire.available());
  while (Wire.available()) { // slave may send less than requested
    byte c = Wire.read(); // receive a byte as character
    Serial.println(c);         // print the character
  }

  /*
      WRITING SETTINGS IN CAPSENSE
      - Write via I2C all the commands necessary to configure Capsense. I think the idea here is to have a conditional configuration option in the firmware and only enter this step if newer configuration needs to be loaded. All values are stored in internal flash memory so this could be bypassed in final firmware.
      - Write ‘3’ to CTRL_CMD in order to generate CRC that is written automatically by the chip to CALC_CRC
      - After 220ms or more read CRC from CALC_CRC
      - Write it to CONFIG_CRC
      - Write ‘2’ to CTRL_CMD to compare current CRC and stored one in CALC_CRC (they are the same) and write in internal flash.
      - Wait 220ms or more for command to finish.

  */


  // Send 0x03 to calculate de CRC from the changed configuration.
  Wire.beginTransmission(I2C_ADDR);
  Wire.write(CTRL_CMD);
  Wire.write(0x03);
  Wire.endTransmission();

  // This delay is important
  delay(300);

  // Read CRC calculated from the 0x03 command sent
  byte crc[2] = {0, 0};
  int i = 0;
  Wire.beginTransmission(I2C_ADDR);
  Wire.write(CALC_CRC);
  Wire.endTransmission();

  Wire.requestFrom(I2C_ADDR, 2);
  while (Wire.available()) { // slave may send less than requested
    byte c = Wire.read();
    Serial.println(c);         // print the character
    crc[i] = c; // receive a byte as character
    i++;
  }
  Wire.endTransmission();

  // Write CRC to CONFIG_CRC
  Wire.beginTransmission(I2C_ADDR);
  Wire.write(CONFIG_CRC);
  Wire.write(crc, 2);
  Wire.endTransmission();

  // Send 0x02 to calculate de CRC from the changed configuration.
  Wire.beginTransmission(I2C_ADDR);
  Wire.write(CTRL_CMD);
  Wire.write(0x02);
  Wire.endTransmission();
  delay(300);

  int result = 2;

  while (result != 0) {
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(CTRL_CMD_STATUS);
    Wire.endTransmission();

    Wire.requestFrom(I2C_ADDR, 1);
    result = Wire.read();
    Serial.print("result CTRL_CMD_STATUS 1: ");
    Serial.println(result);
    if (result == 1) {
      Wire.beginTransmission(I2C_ADDR);
      Wire.write(CTRL_CMD_ERROR);
      Wire.endTransmission();

      Wire.requestFrom(I2C_ADDR, 1);
      result = Wire.read();
      Serial.print("result CTRL_CMD_ERROR 1: ");
      Serial.println(result);
      Wire.endTransmission();
    }
    Wire.endTransmission();
  }

  Wire.beginTransmission(I2C_ADDR);
  Wire.write(CTRL_CMD);
  Wire.write(0xFF);
  Wire.endTransmission();
  delay(300);

  result = 2;

  while (result != 0) {
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(CTRL_CMD_STATUS);
    Wire.endTransmission();

    Wire.requestFrom(I2C_ADDR, 1);
    result = Wire.read();
    Serial.print("result CTRL_CMD_STATUS 2: ");
    Serial.println(result);
    if (result == 1) {
      Wire.beginTransmission(I2C_ADDR);
      Wire.write(CTRL_CMD_ERROR);
      Wire.endTransmission();

      Wire.requestFrom(I2C_ADDR, 1);
      result = Wire.read();
      Serial.print("result CTRL_CMD_ERROR 2: ");
      Serial.println(result);
      Wire.endTransmission();
    }
    Wire.endTransmission();
  }
  delay(500);
}

boolean readTouch() {
  boolean changed = 0;
  byte temp[2] = {0, 0}; int i = 0;
  Wire.beginTransmission(I2C_ADDR);
  Wire.write(BUTTON_STAT);
  Wire.endTransmission();

  Wire.requestFrom(I2C_ADDR, 2);
  while (Wire.available()) { // slave may send less than requested
    //    byte c = Wire.read();
    //    temp[i] = c; // receive a byte as character
    temp[i] = Wire.read();
    i++;
  }
  Wire.endTransmission();

  for (int t = 0; t < 2; t++) {
    if (temp[t] != touch[t]) {
      changed = 1;
      touch[t] = temp[t];
    }
  }
  return changed;
}



void setupSensor()
{
  // 1.) Set the accelerometer range
  lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_2G);
  //lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_4G);
  //lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_8G);
  //lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_16G);

  // 2.) Set the magnetometer sensitivity
  lsm.setupMag(lsm.LSM9DS1_MAGGAIN_4GAUSS);
  //lsm.setupMag(lsm.LSM9DS1_MAGGAIN_8GAUSS);
  //lsm.setupMag(lsm.LSM9DS1_MAGGAIN_12GAUSS);
  //lsm.setupMag(lsm.LSM9DS1_MAGGAIN_16GAUSS);

  // 3.) Setup the gyroscope
  lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_245DPS);
  //lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_500DPS);
  //lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_2000DPS);
}

void scan() {
  Serial.println("\nI2C Scanner");

  byte error, address;
  int nDevices;

  Serial.println("Scanning...");

  nDevices = 0;
  for(address = 1; address < 127; address++ ) 
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address<16) 
        Serial.print("0");
      Serial.print(address,HEX);
      Serial.println("  !");

      nDevices++;
    }
    else if (error==4) 
    {
      Serial.print("Unknown error at address 0x");
      if (address<16) 
        Serial.print("0");
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");
}


void setup()
{
  Serial.begin(115200);

  while (!Serial) {
    delay(1); // will pause Zero, Leonardo, etc until serial console opens
  }

  scan();
  
  Serial.println("\nchecking IMU.....\n\n");

  // Try to initialise and warn if we couldn't detect the chip
  if (!lsm.begin())
  {
    Serial.println("Oops ... unable to initialize the LSM9DS1. Check your wiring!");
    IMU_OK = false;
  }
  else {
    Serial.println("\n*******************");
    Serial.println("Found LSM9DS1 9DOF");
    Serial.println("*******************\n");
    IMU_OK = true;
    // helper to just set the default scaling we want, see above!
    setupSensor();
    delay(2000);
  }

  Serial.println("\n\n Checking Capsense....");
  initCapsense();
  Capsense_OK = true;
  Serial.println("\n*******************");
  Serial.println("Capsense OK");
  Serial.println("*******************\n");
}

void loop()
{
  if (IMU_OK) {
    lsm.read();  /* ask it to read in the data */

    /* Get a new sensor event */
    sensors_event_t a, m, g, temp;

    lsm.getEvent(&a, &m, &g, &temp);

    Serial.print("Accel X: "); Serial.print(a.acceleration.x); Serial.print(" m/s^2");
    Serial.print("\tY: "); Serial.print(a.acceleration.y);     Serial.print(" m/s^2 ");
    Serial.print("\tZ: "); Serial.print(a.acceleration.z);     Serial.println(" m/s^2 ");

    Serial.println();
  }

  if (Capsense_OK) {
    if (readTouch())
    {
      Serial.print("Touch bytes: 0x");
      Serial.print(touch[0], HEX);
      Serial.print(touch[1], HEX);
      Serial.println();
    }
  }
  delay(200);


}
