// Edu Meneses (IDMIL) - 2020
// www.edumeneses.com | www.idmil.org


#include "led.h"


int Led::blink(int intensity, int onTime) { // onTime between 10 and 100 (percent)
  int result = 0;
  if (onTime <= 10) {
    onTime = 10;
  }
  if (onTime > 100) {
    onTime = 100;
  }
  int switchPoint = 100/onTime;
  if ((millis() - Led::timer) > Led::interval) {
    Led::timer = millis();
  }
  if ( (millis() - Led::timer) < Led::interval/switchPoint ) {
    result = intensity;
  } else {
    result = 0;
  }
  return result;
}

int Led::rampUp(int currentValue, int startValue, int endValue) { // plug currentValue in startValue for a 'log' curve
  int result = currentValue;
  if (endValue > currentValue) {
    int rampStep = Led::interval/(endValue - startValue);
    if ((millis() - Led::timer) > rampStep) {
      Led::timer = millis();
      result += 1;
    }
  }
  return result;
}

int Led::rampDown(int currentValue, int startValue, int endValue) { // plug currentValue in startValue for a 'log' curve
  int result = currentValue;
  if (endValue < currentValue) {
    int rampStep = Led::interval/(startValue - endValue);
    if ((millis() - Led::timer) > rampStep) {
      Led::timer = millis();
      result -= 1;
    }
  }
  return result;
}

int Led::cycle(int currentValue, int minValue, int maxValue) {
  int result = currentValue;
  if (Led::n <= 0) {
    Led::n = 1;
  }
  if (millis()-Led::timer > Led::interval) {
    Led::timer = millis();
    Led::n = 1;
  }
  int nMax = 2 * (maxValue - minValue);               // # of steps
  int rampStep = Led::interval / nMax;                // time between steps
  if (millis()-Led::timer > Led::interval/2) {        // ramping down
    if (minValue < currentValue) {
      if ((millis() - Led::timer) > rampStep*Led::n) {
        Led::n++;
        result -= 1;
      }
    }
  } else {                                            // ramping up
    if (maxValue > currentValue) { 
      if ((millis() - Led::timer) > rampStep*Led::n) {
        Led::n++;
        result += 1;
      }
    }
  }
  return result;
}

bool Led::setInterval(int interval) {
  Led::interval = interval;
  return 1;
}