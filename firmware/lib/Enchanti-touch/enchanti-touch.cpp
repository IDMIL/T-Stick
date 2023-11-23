#include "enchanti-touch.h"

// Initialise touch board, 
void EnchantiTouch::initTouch(float num, int threshold, int mode) {
    // Clip number of touch boards to 2
    if (num > 2) {
        num = 2;
    } else if (num < 0) {
        num = 1;
    }

    // Save properties to class
    num_boards = num;
    noise_threshold = threshold;
    boardMode = mode;
    if (boardMode == RAW) {
        baseReg = RAW_REG;
    } else if (boardMode == DIFF) {
        baseReg = DIFF_REG;
    } else if (boardMode == BASELINE) {
        baseReg = BASELINE_REG;
    } else {
        baseReg = DIFF_REG;
    }

    // set touchsize
    touchSize = floor(num_boards * ENCHANTI_BASETOUCHSIZE);
    // enable sensor
    // TODO: send an I2C command back to sensor to start its initialisation process
    running = true;
}

void EnchantiTouch::readTouch(){
    // Read data from all 60 touch buttons
    // Compute buffer length
    int length = 0;
    if (touchSize < ENCHANTI_BASETOUCHSIZE) {
        // Each sensor needs 2 bytes == 120 Byte read
        length = touchSize * 2;
    } else {
        length = ENCHANTI_BASETOUCHSIZE * 2;
    }
    
    // Read touch data from I2C buffer
    readBuffer(main_i2c_addr, baseReg, length);

    // Read auxillary touch board data
    if (num_boards > 1) {
        length = (touchSize - ENCHANTI_BASETOUCHSIZE) * 2;
        readBuffer(aux_i2c_addr, baseReg, length, ENCHANTI_BASETOUCHSIZE); // offset data index to not overwrite main touch board data
    }
}

void EnchantiTouch::cookData() {
    // Get max value, but also use it to check if any touch is pressed
    int instant_maxTouchValue = *std::max_element(data, data+touchSize);

    // Touching the T-Stick or not?
    if (instant_maxTouchValue == 0) {
        touchStatus = 0;
    } else {
        touchStatus = 1;
    }

    // We need a updated maxTouchValue to normalize touch
    maxTouchValue = std::max(maxTouchValue,instant_maxTouchValue);

    // Touch discretize and normalize
    for (int i=0; i < touchSize; i++) {
        touch[i] = data[i] - noise_threshold;
        if(touch[i] < 0) touch[i] = 0; // Make sure above 0

        // Set other data vectors
        if (touch[i] != 0) {
            discreteTouch[i] = 1;
            normTouch[i] = float(data[i]) / float(maxTouchValue);
        } else {
            normTouch[i] = 0;
            discreteTouch[i] = 0;
        }
    }
}

void EnchantiTouch::readBuffer(uint8_t i2c_addr, uint8_t reg, uint8_t length, int offset)
{
    // prepare for data read
    uint16_t value = 0;  
    Wire.beginTransmission(i2c_addr); 
    Wire.write(reg);
    uint8_t last_status = Wire.endTransmission(false);

    Wire.requestFrom(i2c_addr, length); 
    for (int i=0; i < floor(length/2); i++) {
        // Read two bytes for each sensor
        value  = Wire.read();
        value |= (uint16_t)Wire.read() << 8;      // value low byte
        data[i + offset] = value;
    }
}