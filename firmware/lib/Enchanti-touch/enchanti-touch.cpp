#include "enchanti-touch.h"

// Initialise touch board, 
uint8_t EnchantiTouch::initTouch(touch_config enchanti_config) {
    float num = enchanti_config.touchsize / ENCHANTI_BASETOUCHSIZE;
    // Clip number of touch boards to 2
    if (num > 2) {
        num = 2;
        touchSize = 2 * ENCHANTI_BASETOUCHSIZE;
    } else if (num < 0) {
        num = 1;
        touchSize = ENCHANTI_BASETOUCHSIZE;
    } else {
        // set touchsize
        num_boards = num;
        touchSize = floor(num_boards * ENCHANTI_BASETOUCHSIZE);
    }

    // Save properties to class
    num_boards = num;
    noise_threshold = enchanti_config.touch_threshold;
    boardMode = enchanti_config.touch_mode;
    comMode = enchanti_config.comm_mode;

    // Setup SPI if spi mode selected
    if (comMode == COMMS::SPI_MODE) {
        // to use DMA buffer, use these methods to allocate buffer
        spi_master_tx_buf = master.allocDMABuffer(ENCHANTI_BUFFERSIZE);
        spi_master_rx_buf = master.allocDMABuffer(ENCHANTI_BUFFERSIZE);

        // set the DMA buffer
        for (uint32_t i = 0; i < ENCHANTI_BUFFERSIZE; i++) {
            spi_master_tx_buf[i] = i & 0xFF;
        }
        memset(spi_master_rx_buf, 0, ENCHANTI_BUFFERSIZE);

        // intialising the spi bus 
        master.setDataMode(SPI_MODE0);    
        master.setFrequency(spiClk);            
        master.setMaxTransferSize(ENCHANTI_BUFFERSIZE);  
        master.setDutyCyclePos(96);
        // Start bus
        master.begin();
    }

    // // Send configuration data to touch board
    // // Send number of boards
    // Wire.beginTransmission(main_i2c_addr);
    // Wire.write(NUMBOARD_REG);
    // Wire.write(int(num_boards));
    // uint8_t last_status = Wire.endTransmission();

    // // Set up touch mode
    // Wire.beginTransmission(main_i2c_addr);
    // Wire.write(TOUCHMODE_REG);
    // Wire.write(mode);
    // last_status = Wire.endTransmission();    
    // enable sensor
    // TODO: send an I2C command back to sensor to start its initialisation process
    running = true;
    return 1;
}

void EnchantiTouch::readTouch(){
    // Read data from all touch buttons
    if (comMode == COMMS::I2C_MODE) {
        // Compute buffer length
        int length = 0;
        if (touchSize < ENCHANTI_BASETOUCHSIZE) {
            // Each sensor needs 2 bytes == 120 Byte read
            length = touchSize * 2;
        } else {
            length = ENCHANTI_BASETOUCHSIZE * 2;
        }

        // Read touch data from I2C buffer
        readI2CBuffer(main_i2c_addr, baseReg, length);

        // Read auxillary touch board data
        if (num_boards > 1) {
            length = (touchSize - ENCHANTI_BASETOUCHSIZE) * 2;
            readI2CBuffer(aux_i2c_addr, baseReg, length, ENCHANTI_BASETOUCHSIZE); // offset data index to not overwrite main touch board data
        }
    }
    if (comMode == COMMS::SPI_MODE) {
        readSPIBuffer(ENCHANTI_BUFFERSIZE, 2);
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
        if (data[i] < noise_threshold) {
            touch[i] = 0;
        } else {
            touch[i] = data[i] - noise_threshold;
        }

        // Set other data vectors
        if (touch[i] != 0) {
            discreteTouch[i] = 1;
            normTouch[i] = (data[i] *100) / maxTouchValue;
        } else {
            normTouch[i] = 0;
            discreteTouch[i] = 0;
        }
    }
}

void EnchantiTouch::readI2CBuffer(uint8_t i2c_addr, uint8_t reg, uint8_t length, int offset)
{
    // prepare for data read
    uint8_t loc = 0;
    uint16_t value = 0;  
    Wire.beginTransmission(i2c_addr); 
    Wire.write(reg);
    uint8_t last_status = Wire.endTransmission();

    // Read the available data
    Wire.requestFrom(i2c_addr, length); 
    while (Wire.available() >= 2) {
        // Read two bytes for each sensor
        uint8_t lsb  = Wire.read();
        uint8_t msb  = Wire.read();
        value = lsb + (msb << 8);
        if (data[loc + offset] != value) {
            newData = 1;
            data[loc + offset] = value;
        }
        ++loc;
    }
}

void EnchantiTouch::readSPIBuffer(uint16_t length, int offset)
{
    // prepare for data read
    uint8_t loc = 0;
    uint16_t value = 0;  

    // Start transaction
    memset(spi_master_rx_buf, 0, ENCHANTI_BUFFERSIZE);
    const size_t received_bytes = master.transfer(NULL, spi_master_rx_buf, length);

    // Process data
    while (loc < (touchSize*2)) {
        uint8_t lsb  = spi_master_rx_buf[loc+offset];
        ++loc;
        uint8_t msb  = spi_master_rx_buf[loc+offset];
        ++loc;
        value = uint16_t(msb << 8) | uint16_t(lsb);
        if (value < 4097) { // spi occassionally throws junk ignore it
            if (data[int(loc/2)] != value) {
                newData = 1;
            }
            data[int(loc/2)] = value;
        }
    }
}