
// Touch (or button) function

#include "touch.h"


unsigned long IRAM_ATTR millis() {
    return (unsigned long) (esp_timer_get_time() / 1000LL);
}

void Touch::readTouch() {
    uint16_t touchValue;
    touch_pad_read_filtered(Touch::pin, &touchValue);
    Touch::value = touchValue;
    if (touchValue < Touch::threshold) {
        if (!Touch::touch) {
            Touch::touch = true;
            Touch::timer = millis();
        }
        if (millis() - Touch::timer > Touch::holdInterval) {
            Touch::hold = true;
        }
    }
    else if (Touch::hold) {
        Touch::hold = false;
        Touch::touch = false;
        Touch::count = 0;
    }
    else {
        if (Touch::touch) {
            Touch::touch = false;
            Touch::pressTime = millis() - Touch::timer;
            Touch::timer = millis();
            Touch::count++;
        }
    }
    if (!Touch::touch && (millis() - Touch::timer > Touch::countInterval)) {
        switch (Touch::count) {
            case 0:
                Touch::tap = 0;
                Touch::dtap = 0;
                Touch::ttap = 0;
                break;
            case 1: 
                Touch::tap = 1;
                Touch::dtap = 0;
                Touch::ttap = 0;
                break;
            case 2:
                Touch::tap = 0;
                Touch::dtap = 1;
                Touch::ttap = 0;
                break;
            case 3:
                Touch::tap = 0;
                Touch::dtap = 0;
                Touch::ttap = 1;
                break;
        }
        Touch::count = 0;
    }
}

bool Touch::initTouch(void) {
        touch_pad_init();
        touch_pad_config(Touch::pin, Touch::threshold);
        touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V);
        touch_pad_filter_start(Touch::filterPeriod);
    return 1;
}

unsigned int Touch::getCount() {
    return Touch::count;
};

bool Touch::getTouch() {
    return Touch::touch;
}

unsigned int Touch::getValue() {
    return Touch::value;
};

unsigned int Touch::getTap() {
    return Touch::tap;
};

unsigned int Touch::getDTap() {
    return Touch::dtap;
};

unsigned int Touch::getTTap() {
    return Touch::ttap;
};

unsigned int Touch::getThreshold() {
    return Touch::value;
};

unsigned int Touch::setSensitivity(int value) {
    Touch::threshold = value;
    return 1;
};

unsigned int Touch::getPressTime() {
    return Touch::pressTime;
}

bool Touch::getHold() {
    return Touch::hold;
}

unsigned int Touch::getHoldInterval() {
    return Touch::holdInterval;
}

unsigned int Touch::setHoldInterval(int value) {
    Touch::holdInterval = value;
    return 1;
}

