
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
    }
    else {
        if (Button::button) {
            Button::button = false;
            Button::pressTime = millis() - Button::timer;
            Button::timer = millis();
        }
    }
}

bool Button::initButton(int &buttonPin) {
    Button::pin = buttonPin;
    pinMode(Button::pin, INPUT_PULLUP);
    return 1;
}

bool Button::getButton() {
    return Button::button;
}

unsigned int Button::getState() {
    return Button::buttonState;
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

