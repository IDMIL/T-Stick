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
        float mapFloat(float x, float in_min, float in_max, float out_min, float out_max);
    public:
        int initFsr(int &fsr_pin, int offsetValue);
        int readFsr();
        float getValue();
        float getNormValue();
        int getOffset();
        int setOffset(int offsetValue);
};

#endif