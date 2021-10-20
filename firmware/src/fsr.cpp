// FSR function

#include "fsr.h"

byte Fsr::initFsr(byte &fsr_pin, int &offsetValue){
    Fsr::pin = fsr_pin;
    Fsr::offset = offsetValue;
    return 1;
}

byte Fsr::readFsr() {
    Fsr::value = analogRead(Fsr::pin);
    Fsr::normValue = constrain((Fsr::value - Fsr::offset) / (4095 - Fsr::offset), 0, 1);
    return 1;
}

float Fsr::getValue() {
    return Fsr::value;
}

int Fsr::getOffset(){
    return Fsr::offset;
}

byte Fsr::setOffset(int &offsetValue){
    Fsr::offset = offsetValue;
    return 1;
}