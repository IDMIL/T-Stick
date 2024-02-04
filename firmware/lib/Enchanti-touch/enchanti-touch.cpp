#include "enchanti-touch.h"

// Initialise touch board, 
void EnchantiTouch::initTouch(float num, int threshold, int mode, int com_mode) {
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
    comMode = com_mode;

    // Setup SPI if spi mode selected
    if (comMode == SPI_MODE) {
        esp_err_t ret;
        ret = spi_bus_initialize(TOUCH_HOST, &buscfg, SPI_DMA_CH_AUTO);
        ESP_ERROR_CHECK(ret);
        //Attach the LCD to the SPI bus
        ret = spi_bus_add_device(TOUCH_HOST, &devcfg, &spi);
        ESP_ERROR_CHECK(ret);

        // set the DMA buffer
        for (uint32_t i = 0; i < ENCHANTI_BUFFERSIZE; i++) {
            spi_master_tx_buf[i] = i & 0xFF;
        }
        memset(spi_master_rx_buf, 0, ENCHANTI_BUFFERSIZE);
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

    // set touchsize
    touchSize = floor(num_boards * ENCHANTI_BASETOUCHSIZE);
    // enable sensor
    // TODO: send an I2C command back to sensor to start its initialisation process
    running = true;
}

void EnchantiTouch::readTouch(){
    // Read data from all touch buttons
    if (comMode == I2C_MODE) {
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
    if (comMode == SPI_MODE) {
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
        touch[i] = data[i] - noise_threshold;
        if(touch[i] < 0) touch[i] = 0; // Make sure above 0

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
    esp_err_t ret;
    static spi_transaction_t trans;
    memset(&trans, 0, sizeof(spi_transaction_t));

    // Reset rx buffer
    memset(&spi_master_rx_buf, 0, ENCHANTI_BUFFERSIZE);
    
    // Setup properties
    trans.length = length;
    trans.flags = SPI_TRANS_USE_RXDATA;
    trans.rx_buffer = &spi_master_rx_buf;

    // Start transaction
    ret = spi_device_polling_transmit(spi, &trans);

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