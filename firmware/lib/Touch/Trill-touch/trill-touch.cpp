// Include Bela Trill Libraries

#include "trill-touch.h"


uint8_t TrillTouch::initTouch(float num, int threshold, int mode) {
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
        // Set touch properties
        delay(10);
        touchArray[i]->setPrescaler(4);
        delay(10);
        touchArray[i]->updateBaseline();
        delay(10);
        touchArray[i]->setScanSettings(0,9);
        delay(10);
        touchArray[i]->setNoiseThreshold(30);
        delay(10);
        touchArray[i]->setMode(Trill::DIFF);

        // Setup interrupt
        touchArray[i]->setAutoScanInterval(1);
        delay(10);
    }
    running = true;
    return 1;
}

void TrillTouch::readTouch() {
    for (int i=0; i < ceil(num_boards); i++) {
        touchArray[i]->requestRawData();
        for (int k=0; k<30; k++) {
            if (touchArray[i]->rawDataAvailable() > 0) {
                TrillTouch::data[k+(i*TRILL_BASETOUCHSIZE)] = touchArray[i]->rawDataRead();
            }
        }
    }
}

int TrillTouch::getData(int data_index) {

    return TrillTouch::data[data_index];
}

void TrillTouch::cookData() {

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