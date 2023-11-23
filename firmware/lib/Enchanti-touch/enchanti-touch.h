#ifndef ENCHANTITOUCH_H
#define ENCHANTITOUCH_H

#include <Arduino.h>
#include "Wire.h"

#define ENCHANTI_BASETOUCHSIZE 60

class EnchantiTouch
{
    public:
        // Board modes
        enum Mode {
			RAW = 1,
			BASELINE = 2,
			DIFF = 3
		};

        // Register addresses for touch data
        enum regAddr
        {
            RAW_REG         = 0x01, // Register for raw data for button 1
            DIFF_REG        = 0x79, // Register of difference data for button 1 (raw - baseline)
            BASELINE_REG    = 0xF1, // Register of baseline data for button 1 
        };

        // Board Properties
        int boardMode = RAW;
        float num_boards = 1; // number of touch boards, set half numbers to indicate if using only the first touch circuit
        uint8_t main_i2c_addr = 0x30;
        uint8_t aux_i2c_addr = 0x31;
        uint8_t baseReg = RAW_REG;

        // Touch properties
        int noise_threshold = 0;
        int maxTouchValue = 0;
        int touchSize = 60;
        int touchStatus = 0;

        // Touch arrays
        uint16_t data[120];
        uint16_t touch[120];
        float normTouch[120];
        int discreteTouch[120];

        
        // Running or not
        bool running = true;

        // Methods
		void initTouch(float num=1, int threshold=0, int mode=DIFF);
        void readTouch();
        void cookData();

    private:
        void readBuffer(uint8_t i2c_addr, uint8_t reg, uint8_t length, int offset = 0);
};
#endif