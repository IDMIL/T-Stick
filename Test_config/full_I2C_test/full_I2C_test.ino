//
// IDMIL's Capsense configuration firmware
//
// Edu Meneses / Johnty Wang - Feb 2020
//
// Use this code to test the capsense board and IMU
//
// Can hold up to 8 capsense boards (theoretically)
//

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

// Capsense chip: CY8CMBR3116

bool Capsense_OK = false;
bool IMU_OK = false;

// Prints a binary number with following Placeholder Zeros  (Automatic Handling)
#define PRINTBINL(Num) for (int i=0;i<(sizeof(Num)*8);i++) Serial.write(((Num >> i) & 1) == 1 ? '1' : '0'); 

#define BUTTON_STAT 0xAA  // Address to read the status of the sensors (2 bytes)

struct Capsense { 
  byte answer1, answer2;
} capsense;

byte capsense_addresses[5];
byte nCapsenses = 0;
byte touch[5][2]; // up to 5 capsenses (2 bytes per capsense)

Capsense capsenseRequest(byte address,byte request, byte answer_size) {

    byte SYSTEM_STATUS = 0x8A;
    byte answer1, answer2;

    // This first requisition doesn't work,
    // it always return 0xFF (255) for some reason.
      Wire.beginTransmission(address);
      Wire.write(SYSTEM_STATUS);
      Wire.endTransmission();
      Wire.requestFrom(address, 1);
      answer1 = Wire.read();
      Wire.endTransmission();
    
    // Send the proper request to Capsense
      Wire.beginTransmission(address);
      Wire.write(request);
      Wire.endTransmission();   
    // Getting the answers
      Wire.requestFrom(address, answer_size);
      answer1 = Wire.read();
      if (answer_size == 2) {
         answer2 = Wire.read();
      }
      Wire.endTransmission();
    return {answer1, answer2};
}

void initCapsense(byte I2C_ADDR) {
  byte SYSTEM_STATUS = 0x8A;
  byte I2C_ADDR_STORE = 0x51;
  byte FAMILY_ID = 0x8F; // Must be 154
  byte DEVICE_ID = 0x90; // Should return 0xA05 (returns 2 bytes)

  Serial.print("\nChecking capsense config (0x"); Serial.print(I2C_ADDR, HEX); Serial.println("):");
  // Asking for Capsense System Status
    Serial.print("- Send a request: Begin Transmission to address 0x"); Serial.print(I2C_ADDR, HEX); 
    Serial.print(" ("); Serial.print(I2C_ADDR); Serial.println(")");
    Serial.println("  Asking for the SYSTEM_STATUS..."); 
    Capsense capsense = capsenseRequest(I2C_ADDR, SYSTEM_STATUS, 1);
    if (capsense.answer1 == 0) {
        Serial.println("  A configuration other than the factory default configuration is loaded");
    } else if (capsense.answer1 == 1){
        Serial.println("  The factory default configuration is loaded");
    } else {
        Serial.print("  Unknown error (weird capsense response): SYSTEM_STATUS: "); Serial.println(capsense.answer1);
    }

    // Asking for Capsense I2C Address
    Serial.print("- Send a request: Begin Transmission to address 0x"); Serial.print(I2C_ADDR, HEX); 
    Serial.print(" ("); Serial.print(I2C_ADDR); Serial.println(")");
    Serial.println("  Asking for the stored I2C_ADDR..."); 
     capsense = capsenseRequest(I2C_ADDR, I2C_ADDR_STORE, 1);
    Serial.print("  Current I2C_ADDR is 0x");
    Serial.print(capsense.answer1, HEX);
    Serial.print(" (");
    Serial.print(capsense.answer1);
    Serial.println(")");
    
    // Asking for Capsense Family ID
    Serial.print("- Send a request: Begin Transmission to address 0x"); Serial.print(I2C_ADDR, HEX); 
    Serial.print(" ("); Serial.print(I2C_ADDR); Serial.println(")");
    Serial.println("  Asking for the FAMILY_ID..."); 
     capsense = capsenseRequest(I2C_ADDR, FAMILY_ID, 1);
    if (capsense.answer1 == 154) {
        Serial.print("  Correct FAMILY_ID found: ");
    } else {
        Serial.print("  INCORRECT FAMILY_ID found: ");
    }
    Serial.println(capsense.answer1);

    // Asking for Capsense Device ID
    Serial.print("- Send a request: Begin Transmission to address 0x"); Serial.print(I2C_ADDR, HEX); 
    Serial.print(" ("); Serial.print(I2C_ADDR); Serial.println(")");
    Serial.println("  Asking for the DEVICE_ID..."); 
     capsense = capsenseRequest(I2C_ADDR, DEVICE_ID, 2);
    if (capsense.answer1 == 5 && capsense.answer2 == 10) {
        Serial.print("  Correct DEVICE_ID found: ");
    } else {
        Serial.print("  INCORRECT DEVICE_ID found: ");
    }
    Serial.print(capsense.answer1); Serial.print(" "); Serial.println(capsense.answer2);

}

void capsense_scan() {
  byte I2C_ADDR;
  byte FAMILY_ID = 0x8F; // Must be 154
  byte address[8];
  
  Serial.println("Scanning for CY8CMBR3116 Capsense boards...");

  for(I2C_ADDR = 1; I2C_ADDR < 127; I2C_ADDR++ ) {
    // This scanner requests the device falily ID
    // The CY8CMBR3116 chip should return 154
    Capsense capsense = capsenseRequest(I2C_ADDR, FAMILY_ID, 1);
    if (capsense.answer1 == 154) {
      Serial.print("I2C device found at address 0x");
      if (I2C_ADDR<16) {Serial.print("0");}
      Serial.print(I2C_ADDR,HEX);
      Serial.println("  !");
      capsense_addresses[nCapsenses] = I2C_ADDR;
      nCapsenses++;
    }
  }
  if (nCapsenses == 0) {
    Serial.println("\n\nOops ... unable to initialize IDMIL's Capsense. Check your wiring/board!\n\n");
  }
  else {
    Capsense_OK = true;
    Serial.println("\n*******************");
    Serial.println("Capsense OK");
    Serial.println("*******************\n");
  }
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
    Serial.println("I2C scanner done\n");
}



void setup()
{
  Serial.begin(115200);

  while (!Serial) {
    delay(1); // will pause Zero, Leonardo, etc until serial console opens
  }

  Wire.begin();
  Wire.setClock(400000);

  // Look for any I2C device (does not work with IDMIL's capsense
  scan();

  // Look for Capsense boards and return their addresses
  capsense_scan(); 

    // Check the capsense(s) stats
  for (byte i=0; i < nCapsenses; i++) {
    if (capsense_addresses[i] != 0) {
      initCapsense(capsense_addresses[i]);
    }
  }

  delay(500);
  
  Serial.println("\nchecking IMU.....\n");

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
    delay(500);
  }

}

void loop()
{
  if (IMU_OK) {
    lsm.read();  /* ask it to read in the data */

    /* Get a new sensor event */
    sensors_event_t a, m, g, temp;

    lsm.getEvent(&a, &m, &g, &temp);

    Serial.print("Accel: X="); 
    if (a.acceleration.x >=0) {Serial.print("+");} else {Serial.print("-");}
    if (a.acceleration.x <10 && a.acceleration.x > -10 ) {Serial.print("0");}
    Serial.print(abs(a.acceleration.x));
    
    Serial.print(", Y="); 
    if (a.acceleration.y >=0) {Serial.print("+");} else {Serial.print("-");}
    if (a.acceleration.y <10 && a.acceleration.y > -10 ) {Serial.print("0");}
    Serial.print(abs(a.acceleration.y));

    Serial.print(", Z="); 
    if (a.acceleration.z >=0) {Serial.print("+");} else {Serial.print("-");}
    if (a.acceleration.z <10 && a.acceleration.z > -10 ) {Serial.print("0");}
    Serial.print(abs(a.acceleration.z));
  }

  if (Capsense_OK) {
    // Read capsense touch data
    for (byte i=0; i < nCapsenses; i++) {
        capsense = capsenseRequest(capsense_addresses[i],BUTTON_STAT, 2);
        touch[i][0] = capsense.answer1;
        touch[i][1] = capsense.answer2;
    }
    // Print Capsense touch data
    Serial.print(" | Touch sensor: [ ");
    for (byte k=0; k < nCapsenses; k++) {
      PRINTBINL(touch[k][0]); PRINTBINL(touch[k][1]);
    }
    Serial.print("]");
  }
  
  Serial.println();
  
  delay(200);
}
