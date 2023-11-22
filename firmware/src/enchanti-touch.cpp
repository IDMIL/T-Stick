#include "tstick_touch.h"

#define BASETOUCHSIZE 60

// Initialise touch board, 
void TouchBoard::init(float num, int threshold, int mode=DIFF) {
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
        baseReg = RAW_REG;
    }

    // set touchsize
    touchSize = num_boards * BASETOUCHSIZE;
}

void TouchBoard::readTouch(){
    // Read data from all 60 touch buttons
    // Compute buffer length
    int length = 0;
    if (touchSize < BASETOUCHSIZE) {
        // Each sensor needs 2 bytes == 120 Byte read
        length = touchSize * 2;
    } else {
        length = BASETOUCHSIZE * 2;
    }
    
    // Read touch data from I2C buffer
    readBuffer(main_i2c_addr, baseReg, length);

    // Read auxillary touch board data
    if (num_boards > 1) {
        length = (touchSize - BASETOUCHSIZE) * 2;
        readBuffer(aux_i2c_addr, baseReg, length, BASETOUCHSIZE); // offset data index to not overwrite main touch board data
    }
}

void TouchBoard::cookTouch() {
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
        if (data[i] != 0) {
            discreteData[i] = 1;
            normalisedData[i] = float(data[i]) / float(maxTouchValue);
        } else {
            normTouch[i] = 0;
            discreteData[i] = 0;
        }
    }
}

void TouchBoard::readBuffer(int i2c_addr, uint8_t reg, uint8_t length, int offset = 0)
{
    // prepare for data read
    uint16_t value = 0;  
    Wire.beginTransmission(i2c_addr); 
    Wire.write(reg);
    uint8_t last_status = Wire.endTransmission(false);

    Wire.requestFrom(i2c_addr, length); 
    for (int i=0; i < length; i++) {
        // Read two bytes for each sensor
        value  = Wire.read();
        value |= (uint16_t)Wire.read() << 8;      // value low byte
        data[i + offset] = value;
    }
}