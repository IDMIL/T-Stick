//////////////////////////
// Capsense Definitions //
//////////////////////////

// This firmware works with up to 4 IDMIL capsenses using I2C. It you want to use more
// capsenses, either change the code to use SPI or change the related array sizes to
// accomodate more readings: 
// capsense_addresses, RawData.touch, LastState.brushUp, LastState.brushDown, Tstick.touchMask

// CY8CMBR3116

#include "capsense.h"

#define BUTTON_STAT 0xAA  // Address to read the status of the sensors (2 bytes)

void Capsense::capsenseRequest(uint8_t address,uint8_t request, uint8_t answer_size) {

    uint8_t SYSTEM_STATUS = 0x8A;

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
}

void Capsense::initCapsense(uint8_t I2C_ADDR) {
  uint8_t SYSTEM_STATUS = 0x8A;
  uint8_t I2C_ADDR_STORE = 0x51;
  uint8_t FAMILY_ID = 0x8F; // Must be 154
  uint8_t DEVICE_ID = 0x90; // Should return 0xA05 (returns 2 bytes)

  printf("\nChecking capsense config (0x"); printf("%x",I2C_ADDR); printf("):\n");
  // Asking for Capsense System Status
    printf("- Send a request: Begin Transmission to address 0x%x",I2C_ADDR); 
    printf(" (%u)\n",I2C_ADDR);
    printf("  Asking for the SYSTEM_STATUS...\n"); 
    capsenseRequest(I2C_ADDR, SYSTEM_STATUS, 1);
    if (answer1 == 0) {
        printf("  A configuration other than the factory default configuration is loaded\n");
    } else if (answer1 == 1){
        printf("  The factory default configuration is loaded\n");
    } else {
        printf("  Unknown error (weird capsense response): SYSTEM_STATUS: %u\n",answer1);
    }

    // Asking for Capsense I2C Address
    printf("- Send a request: Begin Transmission to address 0x%x (%u)\n",I2C_ADDR, I2C_ADDR);
    printf("  Asking for the stored I2C_ADDR...\n"); 
    capsenseRequest(I2C_ADDR, I2C_ADDR_STORE, 1);
    printf("  Current I2C_ADDR is 0x%x (%u)\n",answer1, answer1);
    
    // Asking for Capsense Family ID
    printf("- Send a request: Begin Transmission to address 0x%x (%u)\n", I2C_ADDR, I2C_ADDR);
    printf("  Asking for the FAMILY_ID...\n"); 
    capsenseRequest(I2C_ADDR, FAMILY_ID, 1);
    if (answer1 == 154) {
        printf("  Correct FAMILY_ID found: ");
    } else {
        printf("  INCORRECT FAMILY_ID found: ");
    }
    printf("%u\n",answer1);

    // Asking for Capsense Device ID
    printf("- Send a request: Begin Transmission to address 0x%x (%u)\n", I2C_ADDR, I2C_ADDR);
    printf("  Asking for the DEVICE_ID...\n"); 
    capsenseRequest(I2C_ADDR, DEVICE_ID, 2);
    if (answer1 == 5 && answer2 == 10) {
        printf("  Correct DEVICE_ID found: ");
    } else {
        printf("  INCORRECT DEVICE_ID found: ");
    }
    printf("%u %u\n",answer1, answer2);
}

uint8_t Capsense::initTouch(touch_config idmilTouch_config) {
  uint8_t I2C_ADDR;
  uint8_t FAMILY_ID = 0x8F; // Must be 154
  uint8_t address[8];
  
  printf("Scanning for CY8CMBR3116 Capsense boards...\n");

  printf("    checking default capsense I2C_ADDR (37)\n");
  I2C_ADDR = 0x37;
  capsenseRequest(I2C_ADDR, FAMILY_ID, 1);
  if (answer1 == 154) {
      printf("I2C device found at address 0x");
      if (I2C_ADDR<16) {printf("0");}
      printf("%x",I2C_ADDR);
      printf("  !\n");
      capsense_addresses[nCapsenses] = I2C_ADDR;
      nCapsenses++;
  }

  printf("    checking alternative soprano capsense second I2C_ADDR (38)\n");
  I2C_ADDR = 0x38;
  capsenseRequest(I2C_ADDR, FAMILY_ID, 1);
  if (answer1 == 154) {
      printf("I2C device found at address 0x");
      if (I2C_ADDR<16) {printf("0");}
      printf("%x",I2C_ADDR);
      printf("  !\n");
      capsense_addresses[nCapsenses] = I2C_ADDR;
      nCapsenses++;
  }

  // if no capsense found in default addresses, try all possible option
  // (this process is slow)
  if (nCapsenses == 0) {
    for(I2C_ADDR = 1; I2C_ADDR < 127; I2C_ADDR++ ) {
      // This scanner requests the device's ID (I2C address)
      // The CY8CMBR3116 chip should return 154
      capsenseRequest(I2C_ADDR, FAMILY_ID, 1);
      if (answer1 == 154) {
        printf("I2C device found at address 0x");
        if (I2C_ADDR<16) {printf("0");}
        printf("%x",I2C_ADDR);
        printf("  !\n");
        capsense_addresses[nCapsenses] = I2C_ADDR;
        nCapsenses++;
      }
    }
  }
  if (nCapsenses == 0) {
    printf("\n\nOops ... unable to initialize IDMIL's Capsense. Check your wiring/board!\n\n\n");
    return 0;
  }
  else {
    printf("Capsense OK\n\n");
    return 1;
  }
  touchStripsSize = nCapsenses*16;
  touchSize = idmilTouch_config.touchsize;
}

int Capsense::getData(int data_index) {
    return Capsense::touch[data_index];
}

void Capsense::readTouch() {
    // Read capsense touch data
    for (int i=0; i < nCapsenses; ++i) {
        capsenseRequest(capsense_addresses[i],BUTTON_STAT, 2);
        data[i*2] = answer1;
        data[(i*2)+1] = answer2;
    }
}

void Capsense::cookData() {
    // process capsense data
    for (int i=0; i < touchStripsSize; ++i) {
        int tmp = bitReadRightToLeft(data[i/8],(i%8));
        if (tmp != touch[i]) {
          newData = 1;
        }
        touch[i] = bitReadRightToLeft(data[i/8],(i%8));
    }
}

int Capsense::bitReadLeftToRight (int number, int position, int type_size) {
    // position from 0 to X, from left (msb) to right (lsb)
    // will return 0 if try to read beyond the last bit
    // use type_size to set range in types bigger than tored data
    // E.g., reading 8 bit data stored on int (4 bytes, or 32 bits, in C++)

    int size = type_size - 1; 
    if ( size < position) {
        return 0;
    }
    int output = (number >> (size-position)) & 1;
    return output;
}

int Capsense::bitReadRightToLeft (int number, int position) {
    // position from 0 to X, from RIGHT (lsb) To LEFT (msb)
    // will return 0 if try to read beyond the last bit

    int output = (number >> position) & 1;
    return output;
}

void Capsense::reorderCapsense (int *origArray, uint8_t arraySize) {
  uint8_t tempArray[arraySize];
  uint8_t order[64] = {
    8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,
    24,25,26,27,28,29,30,31,16,17,18,19,20,21,22,23,
    40,41,42,43,44,45,46,47,32,33,34,35,36,37,38,39,
    56,57,58,59,60,61,62,63,48,49,50,51,52,53,54,55
  };
  for (uint8_t i=0; i < sizeof(tempArray)/sizeof(tempArray[0]); ++i) {
    tempArray[i] = origArray[order[i]];
  }
  memcpy(origArray, tempArray, sizeof(tempArray));
}