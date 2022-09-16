
// Button (or button) function

#include "button.h"

void Button::readButton() {
    buttonState = !digitalRead(pin);
    if (buttonState) {
        if (!Button::button) {
            Button::button = true;
            Button::timer = millis();
        }
        if (millis() - Button::timer > Button::holdInterval) {
            Button::hold = true;
        }
    }
    else if (Button::hold) {
        Button::hold = false;
        Button::button = false;
        Button::count = 0;
    }
    else {
        if (Button::button) {
            Button::button = false;
            Button::pressTime = millis() - Button::timer;
            Button::timer = millis();
            Button::count++;
        }
    }
    if (!Button::button && (millis() - Button::timer > Button::countInterval)) {
        switch (Button::count) {
            case 0:
                Button::tap = 0;
                Button::dtap = 0;
                Button::ttap = 0;
                break;
            case 1: 
                Button::tap = 1;
                Button::dtap = 0;
                Button::ttap = 0;
                break;
            case 2:
                Button::tap = 0;
                Button::dtap = 1;
                Button::ttap = 0;
                break;
            case 3:
                Button::tap = 0;
                Button::dtap = 0;
                Button::ttap = 1;
                break;
        }
        Button::count = 0;
    }
}

bool Button::initButton(byte &buttonPin) {
    Button::pin = buttonPin;
    pinMode(Button::pin, INPUT_PULLUP);
    return 1;
}

unsigned int Button::getCount() {
    return Button::count;
};

bool Button::getButton() {
    return Button::button;
}

unsigned int Button::getState() {
    return Button::buttonState;
};

unsigned int Button::getTap() {
    return Button::tap;
};

unsigned int Button::getDTap() {
    return Button::dtap;
};

unsigned int Button::getTTap() {
    return Button::ttap;
};

unsigned int Button::getPressTime() {
    return Button::pressTime;
}

bool Button::getHold() {
    return Button::hold;
}

unsigned int Button::getHoldInterval() {
    return Button::holdInterval;
}

unsigned int Button::setHoldInterval(int value) {
    Button::holdInterval = value;
    return 1;
}

