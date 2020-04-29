#include "TStick.h"

//////////////////////////
// Capsense Definitions //
//////////////////////////

// This firmware works with up to 4 IDMIL capsenses using I2C. It you want to use more
// capsenses, either change the code to use SPI or change the related array sizes to
// accomodate more readings: 
// capsense_addresses, RawData.touch, LastState.brushUp, LastState.brushDown, Tstick.touchMask

// CY8CMBR3116

// #define BUTTON_STAT 0xAA  // Address to read the status of the sensors (2 bytes)

byte capsense_addresses[4]; // max 4 capsenses
byte nCapsenses = 0;
byte touchStripsSize;

Capsense capsenseRequest(uint8_t address,uint8_t request, uint8_t answer_size) {

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
    // This scanner requests the device's ID (I2C address)
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
    Serial.println("Capsense OK\n");
  }
  touchStripsSize = nCapsenses*16;
}

