// Include Bela Trill Libraries

#include "touch.h"


uint8_t Touch::initTouch(float num, int threshold, int mode) {
    // Clip number of touch boards to 4
    if (num > 4) {
        num = 4;
    } else if (num < 0) {
        num = 1;
    }

    // Compute touch size
    touchSize = floor(num * TRILL_BASETOUCHSIZE);
    num_boards = num;

    // Initialise Sensors
    for (int i=0; i < ceil(num_boards); i++) {
        int ret = touchArray[i]->setup(Trill::TRILL_CRAFT, main_i2c_addr + i);
        if(ret != 0) {
            Serial.println("failed to initialise trillSensor1");
            Serial.print("Error code: ");
            Serial.println(ret);
            running = false;
            return 0;
        }
        delay(10);
        touchArray[i]->setPrescaler(2);
        delay(10);
        touchArray[i]->updateBaseline();
        delay(10);
        touchArray[i]->setScanSettings(0,9);
        delay(10);
        touchArray[i]->setNoiseThreshold(30);
        delay(10);
        touchArray[i]->setMode(Trill::DIFF);
    }
    running = true;
    return 1;
}

void Touch::readTouch() {
    for (int i; i < ceil(num_boards); i++) {
        touchArray[i]->requestRawData();
        for (int k=0; k<30; k++) {
            if (touchArray[i]->rawDataAvailable() > 0) {
                Touch::data[k+(i*TRILL_BASETOUCHSIZE)] = touchArray[i]->rawDataRead();
            }
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
            discreteTouch[i] = 1;
            touch[i] = data[i];
            normTouch[i] = (data[i] *100) / maxTouchValue;
        } else {
            touch[i] = 0;
            normTouch[i] = 0;
            discreteTouch[i] = 0;
        }
    }
}