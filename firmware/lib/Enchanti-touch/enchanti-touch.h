#ifndef ENCHANTITOUCH_H
#define ENCHANTITOUCH_H

#include <Arduino.h>
#include "Wire.h"
#include <iostream>
#include "driver/spi_master.h"
#include "esp_system.h"

#define ENCHANTI_BASETOUCHSIZE 60
#define ENCHANTI_BUFFERSIZE 256
#define ENCHANTI_MAXBUFFERSIZE 1024

// 
#define TOUCH_HOST    SPI2_HOST
// SPI Pins
#define PIN_NUM_CS 10
#define PIN_NUM_MISO 13
#define PIN_NUM_MOSI 11
#define PIN_NUM_CLK 12

class EnchantiTouch
{
    public:
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

        // Register addresses for touch data
        enum regAddr
        {
            BOARDMODE_REG   = 1, // Register for mode of the board
            NUMBOARD_REG    = 2, // Register for number of touch boards
            TOUCHMODE_REG   = 3, // Register for the touch mode
            DATA_REG        = 4, // Register of difference data for button 1 (raw - baseline)
        };
        // SPI   
        uint8_t spi_master_tx_buf[ENCHANTI_BUFFERSIZE];
        uint8_t spi_master_rx_buf[ENCHANTI_BUFFERSIZE];

        // Include SPI
        spi_device_handle_t spi;
        spi_bus_config_t buscfg = {
            .mosi_io_num = PIN_NUM_MOSI,
            .miso_io_num = PIN_NUM_MISO,
            .sclk_io_num = PIN_NUM_CLK,
            .quadwp_io_num = -1,
            .quadhd_io_num = -1,
            .max_transfer_sz = ENCHANTI_MAXBUFFERSIZE
        };

        // Interface config
        spi_device_interface_config_t devcfg = {
                .mode = 0,                              //SPI mode 0
                .clock_speed_hz = SPI_MASTER_FREQ_8M,   //SPI Clk
                .spics_io_num = PIN_NUM_CS,             //CS pin
                .queue_size = 1,                        //We want to be able to queue 7 transactions at a time
        };

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
        int maxTouchValue = 0;
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
		void initTouch(float num=1, int threshold=0, int mode=DIFF, int com_mode=SPI_MODE);
        void readTouch();
        void cookData();

    private:
        void readI2CBuffer(uint8_t i2c_addr, uint8_t reg, uint8_t length, int offset = 0);
        void readSPIBuffer(uint16_t length, int offset = 0);
};
#endif