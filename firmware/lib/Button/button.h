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
        int pin = 15;
        int buttonState = 0;
        bool button = false;
        bool hold;
        unsigned int holdInterval = 5000;
        unsigned int filterPeriod = 10;
        long timer;
        unsigned long pressTime;
    public:
        bool initButton(int &buttonPin);
        void readButton();
        bool getButton();
        unsigned int getPressTime();
        unsigned int getState();
        bool getHold();
        unsigned int getHoldInterval();
        unsigned int setHoldInterval(int value);
};

#endif