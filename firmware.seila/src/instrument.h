// Some high-level gestural descriptors of the T-Stick
// using the IMU
// Edu Meneses - 2020 (IDMIL)

#ifndef INSTRUMENT_H
#define INSTRUMENT_H

#include <Arduino.h>

#include <deque>
#include <cmath>        // std::abs
#include <algorithm>    // std::min_element, std::max_element

class Instrument {
    private:
    int queueAmount = 5; // # of values stored
    std::deque<float> gyroXArray; // store last values
    std::deque<float> gyroYArray;
    std::deque<float> gyroZArray;
    int arraycounter = 0;
    const int leakyShakeFreq = 10;
    unsigned long leakyShakeTimerX;
    unsigned long leakyShakeTimerY;
    unsigned long leakyShakeTimerZ;
    float shakeX;
    float shakeY;
    float shakeZ;
    float jabX;
    float jabY;
    float jabZ;
    int jabThreshold = 10;
    public:
    float leakyIntegrator (float reading, float old_value, float leak, int frequency, unsigned long& timer);
    void updateInstrument (float gyroX, float gyroY, float gyroZ);
    float getShakeX();
    float getShakeY();
    float getShakeZ();
    float getJabX();
    float getJabY();
    float getJabZ();
};

#endif