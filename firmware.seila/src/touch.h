// Touch (or button) function

#ifndef TOUCH_H
#define TOUCH_H

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/touch_pad.h"
#include "esp_log.h"

class Touch {
    private:
        touch_pad_t pin = TOUCH_PAD_NUM3; // pin 15 on Wemos D32 Pro and TinyPICO 
        unsigned int count;
        unsigned int countInterval = 200;
        uint16_t value;
        bool touch = false;
        unsigned int tap;
        unsigned int dtap;
        unsigned int ttap;
        bool hold;
        unsigned int holdInterval = 5000;
        unsigned int filterPeriod = 10;
        long timer;
        unsigned long pressTime;
        unsigned int threshold = 870;
    public:
        bool initTouch(void);
        void readTouch();
        unsigned int getCount();
        bool getTouch();
        unsigned int getPressTime();
        unsigned int getValue();
        unsigned int getTap();
        unsigned int getDTap();
        unsigned int getTTap();
        bool getHold();
        unsigned int getHoldInterval();
        unsigned int setHoldInterval(int value);
        unsigned int getThreshold();
        unsigned int setSensitivity(int value);
};

#endif