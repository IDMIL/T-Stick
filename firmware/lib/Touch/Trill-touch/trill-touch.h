// Include Bela Trill Libraries


#ifndef TRILLTOUCH_H
#define TRILLTOUCH_H

#include <Arduino.h>
#include <Trill.h>
#include <vector>

#define TRILL_BASETOUCHSIZE 30

class TrillTouch {
    public:
        uint8_t initTouch(float num=1, int threshold=0, int mode=Trill::DIFF);
        void readTouch();
        int getData(int data_index);
        
        // trill board properties
        uint8_t main_i2c_addr = 0x30;
        Trill trillSensor1;      // for Trill Craft
        Trill trillSensor2;      // for Trill Craft
        Trill trillSensor3;      // for Trill Craft
        Trill trillSensor4;      // for Trill Craft
        std::vector<Trill *> touchArray = {&trillSensor1, &trillSensor2, &trillSensor3, &trillSensor4}; // array of touch data

        int data[120];
        byte touchStatus = 0;
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