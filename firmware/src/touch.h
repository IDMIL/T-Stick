// Include Bela Trill Libraries


#ifndef TOUCH_H
#define TOUCH_H

#include <Arduino.h>
#include <Trill.h>

class Touch {
    public:
        uint8_t initTouch();
        void readTouch();
        int getData(int data_index);
        Trill trillSensor;      // for Trill Craft
        int data[30];
        byte touchStatus = 0;
        int touch[30];          // /instrument/touch/touch, i..., 0 or 1, ... (1 per stripe)
        float normTouch[30];    // /instrument/touch/norm, i..., 0--1, ... (1 per stripe)
        int discreteTouch[30];    // /instrument/touch/raw, i..., 0--1, ... (1 per stripe)
        void cookData();
        int touchSize = 30;
    private:
        int maxTouchValue = 50;
};

#endif