#ifndef ENCHANTITOUCH_H
#define ENCHANTITOUCH_H

#include <Arduino.h>
#include "Wire.h"
#include <ESP32DMASPIMaster.h>
#include <iostream>
#include <../touch-common.h>

#define ENCHANTI_BASETOUCHSIZE 60
#define ENCHANTI_BUFFERSIZE 256
#define ENCHANTI_MAXBUFFERSIZE 1024

// SPI Pins
#define CS 10
#define MISO 13
#define MOSI 11
#define CLK 12

// Board modes
enum Mode {
    RAW = 1,
    BASELINE = 2,
    DIFF = 3
};
// Board modes
enum COMMS {
    SPI_MODE = 1,
    I2C_MODE = 2
};

// Default config
static touch_config default_config = {
    -1, // default use the trill craft device
    ENCHANTI_BASETOUCHSIZE, // default touch size
    0, // noise threshold
    Mode::DIFF, // touch processing mode
    COMMS::I2C_MODE, // comm mode 
};

class EnchantiTouch
{
    public:
        // Register addresses for touch data
        enum regAddr
        {
            BOARDMODE_REG   = 1, // Register for mode of the board
            NUMBOARD_REG    = 2, // Register for number of touch boards
            TOUCHMODE_REG   = 3, // Register for the touch mode
            DATA_REG        = 4, // Register of difference data for button 1 (raw - baseline)
        };

        // Include SPI
        ESP32DMASPI::Master master;
        const int spiClk = 2000000;
        uint8_t* spi_master_tx_buf;
        uint8_t* spi_master_rx_buf;

        // Board Properties
        int comMode = SPI_MODE;
        int boardMode = RAW;
        int newData = 0;
        float num_boards = 1; // number of touch boards, set half numbers to indicate if using only the first touch circuit
        uint8_t main_i2c_addr = 0x1E;
        uint8_t aux_i2c_addr = 0x1F;
        uint8_t baseReg = DATA_REG;

        // Touch properties
        int noise_threshold = 0;
        int maxTouchValue = 1;
        int touchSize = 60;
        int touchStatus = 0;

        // Touch arrays
        uint16_t data[120];
        uint16_t touch[120];
        int normTouch[120];
        int discreteTouch[120];

        // Time since last touch
        int scantimer = 0;
        int i2ctimer = 0;
        int scantime = 0;
        int polltime = 0;

        
        // Running or not
        bool running = true;

        // Methods
		uint8_t initTouch(touch_config enchanti_config);
        void readTouch();
        void cookData();

    private:
        void readI2CBuffer(uint8_t i2c_addr, uint8_t reg, uint8_t length, int offset = 0);
        void readSPIBuffer(uint16_t length, int offset = 0);
};
#endif
