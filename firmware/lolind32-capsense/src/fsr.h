// fsr

#ifndef FSR_H
#define FSR_H

#include <Arduino.h>

class Fsr {
    private:
        int pin;
        float value = 0;
        float normValue = 0;
        float cookedValue = 0;
        int offset = 0;
    public:
        int initFsr(int &fsr_pin, int offsetValue);
        int readFsr();
        float getValue();
        float getNormValue();
        float getCookedValue();
        int getOffset();
        int setOffset(int offsetValue);
};

#endif