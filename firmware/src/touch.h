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
        Trill trillSensor; // for Trill Craft
        int data[30];
    private:
        //int data[30];
        int norm_data[30];
        int trig_data[30];
};

#endif