// FSR function

#include "fsr.h"

int Fsr::initFsr(int &fsr_pin, int offsetValue){
    Fsr::pin = fsr_pin;
    Fsr::offset = offsetValue;
    return 1;
}

int Fsr::readFsr() {
    Fsr::value = analogRead(Fsr::pin);
    Fsr::normValue = mapFloat(Fsr::value, Fsr::offset, 4095, 0, 1);
    return 1;
}

float Fsr::mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
    x = constrain(x, in_min, in_max);
    float result = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    result = constrain(result, out_min, out_max);
    return result;
}

float Fsr::getValue() {
    return Fsr::value;
}

float Fsr::getNormValue() {
    return Fsr::normValue;
}

int Fsr::getOffset(){
    return Fsr::offset;
}

int Fsr::setOffset(int offsetValue){
    Fsr::offset = offsetValue;
    return 1;
}