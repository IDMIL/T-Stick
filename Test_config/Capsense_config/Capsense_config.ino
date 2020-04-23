//
// IDMIL's Capsense configuration firmware
//
// Edu Meneses - Feb 2020
//
// Use this code to test and program the capsense board
// (change I2C address, etc.)
// Can hold up to 5 capsense boards
//
// Capsense chip: CY8CMBR3116
//
//
// IF CHANGING I2C ADDRESS, CONNECT ONLY 1 CAPSENSE
//


// ********************************************************
#define CHANGE_IP false
#define I2C_SET_ADDRESS 0x37 // Default address 0x37
// ********************************************************


#include <Wire.h>

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
    Serial.println("No I2C devices found\n");
  }
  else {
    Serial.println();
  }
}

void configCapsense(byte I2C_ADDR, byte new_address) {
    byte SYSTEM_STATUS = 0x8A;
    byte SENSOR_EN = 0x00;
    byte CTRL_CMD_STATUS = 0x88;
    byte CTRL_CMD_ERROR = 0x89;
    byte FSS_EN = 0x02; // Flanking Sensor Suppression (FSS)
    byte SPO_CFG = 0x4C; //CS15 configuration address
    byte CTRL_CMD = 0x86; // To configure the Capsense
    byte CALC_CRC = 0x94;
    byte CONFIG_CRC = 0x7E;
    byte SENSITIVITY0 = 0x08;
    byte SENSITIVITY1 = 0x09;
    byte SENSITIVITY2 = 0x0a;
    byte SENSITIVITY3 = 0x0b;
    byte infoboard = 0;

    Serial.println("\n\nStarting Capsense configuration");

    // This first requisition doesn't work,
    // it always return 0xFF (255) for some reason.
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(SYSTEM_STATUS);
    Wire.endTransmission();
    Wire.requestFrom(I2C_ADDR, 1);
    infoboard = Wire.read();
    Wire.endTransmission();

    // Address configuration
    Wire.beginTransmission(I2C_ADDR); 
    Wire.write(0x51); // I2C_ADDR recording address
    Serial.print("Setting capsense address to 0x"); Serial.println(new_address, HEX);
    Wire.write(new_address);
    Wire.endTransmission();
    
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
    Wire.write(0x00);
    Wire.write(0x00);
    Wire.endTransmission();
  
    // Special Purpose Output pin configuration
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(SPO_CFG);
    Wire.write(B00010101);
    Wire.endTransmission();

    // Sensitivity
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(SENSITIVITY0);
    Wire.write(B10101010);
    Wire.endTransmission();
  
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(SENSITIVITY1);
    Wire.write(B10101010);
    Wire.endTransmission();
  
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(SENSITIVITY2);
    Wire.write(B10101010);
    Wire.endTransmission();
  
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(SENSITIVITY3);
    Wire.write(B10101010);
    Wire.endTransmission();
  
    /*
      WRITING SETTINGS IN CAPSENSE
      - Write via I2C all the commands necessary to configure Capsense. 
        I think the idea here is to have a conditional configuration option in the firmware 
        and only enter this step if newer configuration needs to be loaded. All values are 
        stored in internal flash memory so this could be bypassed in final firmware.
      - Write ‘3’ to CTRL_CMD in order to generate CRC that is written automatically by the 
        chip to CALC_CRC
      - After 220ms or more read CRC from CALC_CRC
      - Write it to CONFIG_CRC
      - Write ‘2’ to CTRL_CMD to compare current CRC and stored one in CALC_CRC (they are the 
        same) and write in internal flash.
      - Wait 220ms or more for command to finish.
    */
  
    // Send 0x03 to calculate de CRC from the changed configuration.
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(CTRL_CMD);
    Wire.write(0x03);
    Wire.endTransmission();
  
    delay(300); // wait until flash update is complete, at least 220 ms
  
    // Read CRC calculated from the 0x03 command sent
    byte crc[2] = {0, 0};
    int i = 0;
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(CALC_CRC);
    Wire.endTransmission();
  
    Wire.requestFrom(I2C_ADDR, 2);
    while (Wire.available()) { // slave may send less than requested
      byte c = Wire.read();
      Serial.print("Printing CALC_CRC response(s): "); Serial.println(c);         // print the character
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
  
    byte counter = 0;
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
      if (counter >= 255) {
        Serial.println("Capsense connection error (1)");
        break;
      }
      counter += 1;
    }
  
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(CTRL_CMD);
    Wire.write(0xFF);
    Wire.endTransmission();
    delay(300);
  
    result = 2;
  
    counter = 0;
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
      if (counter >= 255) {
        Serial.println("Capsense connection error (2)");
        break;
      }
      counter += 1;
    }
    Serial.println("Capsense configuration finished\n");
    delay(500);
}
  
void setup() {

  Serial.begin(115200);
  while (!Serial); // Leonardo: wait for serial monitor
  Serial.println("\nI2C Capsense CY8CMBR3116 config\n");

  Wire.begin();
  Wire.setClock(400000);

  // Look for Capsense boards and return their addresses
  capsense_scan(); 

  // Check the capsense(s) stats
  for (byte i=0; i < nCapsenses; i++) {
    if (capsense_addresses[i] != 0) {
      initCapsense(capsense_addresses[i]);
    }
  }

  // Config Capsense
  if (CHANGE_IP) {
    configCapsense(capsense_addresses[0], I2C_SET_ADDRESS);
  }
}


void loop() {

  // Read capsense touch data
  for (byte i=0; i < nCapsenses; i++) {
      capsense = capsenseRequest(capsense_addresses[i],BUTTON_STAT, 2);
      touch[i][0] = capsense.answer1;
      touch[i][1] = capsense.answer2;
  }

  // Print Capsense touch data
    for (byte k=0; k < nCapsenses; k++) {
      PRINTBINL(touch[k][0]); PRINTBINL(touch[k][1]);
    }
  Serial.println();
  
  //delay(50);
}
