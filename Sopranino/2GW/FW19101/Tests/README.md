# T-Stick Module Tests

Contains both arduino sketches (may need to check library dependencies) and compiled binaries that can be uploaded directly using the [ESP flash download tool](https://www.espressif.com/en/support/download/other-tools) (Windows only), or the python script [esptool](https://github.com/espressif/esptool).

## Compiling from scratch

Alternatively, you can compile these examples in the Arduino IDE with the following libraries:

- Adafruit LSM9DS1
- Adafruit Sensor

Also, make sure you have the esp32 build system installed with the Arduino IDE!

# MAKE SURE YOU CONNECT THE SENSOR BOARDS IN THE RIGHT ORIENTATION!!!

## IMU-test:
- basic Adafruit LSM9DS1 test example. Spits out IMU data through serial port after initialization

## Capsense:
- Tests the Capsense board by itself. It should work with just the ESP32 board + Capsense board. Once the sketch is running, you should be able to rub your fingers on the pin pads where the copper strips will be connected to, and see the finger touch detected through packed bytes sent via the serial port.

## CapsenseIMU:
- Combined test for both boards.
