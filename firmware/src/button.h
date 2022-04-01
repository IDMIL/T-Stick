// Button (or button) function

#ifndef BUTTON_H
#define BUTTON_H

// #include <stdio.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "esp_log.h"

#include <Arduino.h>

class Button {
    private:
        byte pin = 15;
        unsigned int count;
        unsigned int countInterval = 200;
        byte buttonState = 0;
        bool button = false;
        unsigned int tap;
        unsigned int dtap;
        unsigned int ttap;
        bool hold;
        unsigned int holdInterval = 5000;
        unsigned int filterPeriod = 10;
        long timer;
        unsigned long pressTime;
    public:
        bool initButton(byte &buttonPin);
        void readButton();
        unsigned int getCount();
        bool getButton();
        unsigned int getPressTime();
        unsigned int getState();
        unsigned int getTap();
        unsigned int getDTap();
        unsigned int getTTap();
        bool getHold();
        unsigned int getHoldInterval();
        unsigned int setHoldInterval(int value);
};

#endif