// fsr

#ifndef FSR_H
#define FSR_H

#include <Arduino.h>

class Fsr {
    private:
        byte pin;
        float value = 0;
        float normValue = 0;
        int offset = 0;
    public:
        byte initFsr(byte &fsr_pin, int &offsetValue);
        byte readFsr();
        float getValue();
        float getNormValue();
        int getOffset();
        byte setOffset(int &offsetValue);
};

#endif