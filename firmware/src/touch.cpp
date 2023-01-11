// Include Bela Trill Libraries

#include "touch.h"


uint8_t Touch::initTouch() {
    int ret = trillSensor.setup(Trill::TRILL_CRAFT);
    if(ret != 0) {
        Serial.println("failed to initialise trillSensor");
        Serial.print("Error code: ");
        Serial.println(ret);
        return 0;
    }
    return 1;
}

void Touch::readTouch() {
    trillSensor.requestRawData();
    for (int i=0; i<30; i++) {
        if (trillSensor.rawDataAvailable() > 0) {
            Touch::data[i] = trillSensor.rawDataRead();
        }
    }
}

int Touch::getData(int data_index) {
    return Touch::data[data_index];
}

void Touch::cookData() {

    // Get max value, but also use it to check if any touch is pressed
    int instant_maxTouchValue = *std::max_element(data, data+touchSize);

    // Touching the T-Stick or not?
    if (instant_maxTouchValue == 0) {
        touchStatus = 0;
    } else {
        touchStatus = 1;
    }

    // We need a updated maxTouchValue to normalize touch
    maxTouchValue = std::max(maxTouchValue,instant_maxTouchValue);

    // Touch discretize and normalize
    for (int i=0; i < touchSize; i++) {
        if (data[i] != 0) {
            touch[i] = 1;
            normTouch[i] = (float)data[i] / (float)maxTouchValue;
        } else {
            touch[i] = 0;
        }
    }
}