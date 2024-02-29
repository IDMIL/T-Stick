// Include Bela Trill Libraries
// Touch library for the trill craft board from Bela


#ifndef TRILLTOUCH_H
#define TRILLTOUCH_H

#include <Arduino.h>
#include <Trill.h>
#include <vector>
#include <../touch-common.h>

#define TRILL_BASETOUCHSIZE 30

// Default config for trill
static touch_config default_config = {
    Trill::TRILL_CRAFT, // default use the trill craft device
    TRILL_BASETOUCHSIZE, // default touch size
    0, // noise threshold
    -1, // touch processing mode (not used)
    -1, // comm mode (not used)
};

class TrillTouch {
    public:
        uint8_t initTouch(touch_config trill_config);
        void readTouch();
        int getData(int data_index);
        
        // trill board properties
        uint8_t craft_i2c_addr = 0x30;
        uint8_t flex_i2c_addr = 0x48;
        Trill trillSensor1;      // for Trill Craft
        Trill trillSensor2;      // for Trill Craft
        Trill trillSensor3;      // for Trill Craft
        Trill trillSensor4;      // for Trill Craft
        std::vector<Trill *> touchArray = {&trillSensor1, &trillSensor2, &trillSensor3, &trillSensor4}; // array of touch data

        int data[120];
        byte touchStatus = 0;
        int newData = 0;
        int touch[120];          // /instrument/touch/touch, i..., 0 or 1, ... (1 per stripe)
        float normTouch[120];    // /instrument/touch/norm, i..., 0--1, ... (1 per stripe)
        int discreteTouch[120];    // /instrument/touch/raw, i..., 0--1, ... (1 per stripe)
        void cookData();
        float num_boards = 1; // number of touch boards, set half numbers to indicate if using only half of the touch data
        int touchSize = 30;
        
        // Running or not
        bool running = true;
    private:
        int maxTouchValue = 50;
};

#endif