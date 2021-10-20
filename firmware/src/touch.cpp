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
