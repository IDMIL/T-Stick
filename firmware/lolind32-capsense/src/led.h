// Edu Meneses (IDMIL) - 2020
// www.edumeneses.com | www.idmil.org

#ifndef LED_H
#define LED_H

#include <Arduino.h>

class Led {
  private:
    unsigned long timer = 0;
    unsigned int interval = 1000;
    unsigned int n = 0;
  public:
    int blink(int intensity, int onTime);
    int rampUp(int currentValue, int startValue, int endValue);
    int rampDown(int currentValue, int startValue, int endValue);
    int cycle(int currentValue, int minValue, int maxValue);
    bool setInterval(int interval);
};

#endif