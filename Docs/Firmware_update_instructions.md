# T-Stick 2GW (Firmware version: 200422)

- [First time firmware upload instructions](#first-time-firmware-upload-instructions)
	* [Option 1: using .bin files and esptool.py](#option-1-using-bin-files-and-esptoolpy)
	* [Option 2: Using Arduino IDE](#option-2-using-arduino-ide)
- [Update firmware instructions](#update-firmware-instructions)
- [Other Documentation](#other-documentation)
- [Firmware information](#firmware-information)

##  First time firmware upload instructions:

###  Option 1: using .bin files and esptool.py - NOT AVAILABLE AT THE MOMENT

<blockquote>

This method is easier/faster. It uses [esptool.py](https://github.com/espressif/esptool).

##### Download the [bin files](./bin):

- Download the .bin files located at the bin [folder](./bin)

##### Download [esptool.py](https://github.com/espressif/esptool):

- Download the _esptool.py_ from https://github.com/espressif/esptool. Use the `Download ZIP` option from Github
- Unzip the _esptool-master.zip_ file

##### Download [mkspiffs tool](https://github.com/igrr/mkspiffs):

- Download the mkspiffs tool. Download the latest version for the ESP32 according to your OS at the [release page](https://github.com/igrr/mkspiffs/releases)
- Extract the file
- Copy the [data](./esp32_arduino_19X_19111/data) folder to the _mkspiffs_ folder
- Edit the `/data/config.json` file for the information to match yout T-Stick (serial number, firmware version, color, etc.)

##### Create the .spiffs.bin file (T-Stick configuration file)

- Open a _Terminal_ window (macOS/Linux) or the _Command Prompt_ (Windows)
- Navigate to the _mkspiffs_ folder
- Execute `./mkspiffs -c data -d 5 -b 4096 -p 256 -s 1507328 esp32_arduino_19X_19111.spiffs.bin` (macOS/Linux) or `mkspiffs.exe -c data -d 0 -b 4096 -p 256 -s 1507328 esp32_arduino_19X_19111.spiffs.bin` (Windows)

##### Connect the T-Stick to the computer and check the USB port:

- [Check the T-Stick (ESP32) port in your computer](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/establish-serial-connection.html):
  - For MacOS/Linux:
    - Open a _Terminal_ window
    - Execute the command `ls /dev/cu.*`. The command will return a list of ports in your computer.
    - Plug the T-Stick (USB) and run the command `ls /dev/cu.*` one more time. You can now compare the lists and anotate the T-Stick USB port. Should be something similar to `/dev/cu.wchusbserial1410`, probably with a different number
    - Linux users should also give the currently logged user read and write access the serial port over USB. Check [here](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/establish-serial-connection.html) for more information
  - For Windows:
    - Check the list of identified COM ports in the [Windows Device Manager](https://support.microsoft.com/en-ca/help/4026149/windows-open-device-manager)
    - Plug the T-Stick (USB) and check the list of identified COM ports in the [Windows Device Manager](https://support.microsoft.com/en-ca/help/4026149/windows-open-device-manager) again. The T-Stick port should appear on the list. Anotate the T-Stick USB port, it should be something similar to `COM3` or `COM16`


##### Flash the firmware (.bin files):

- Use _Finder_, _Terminal_, or _File Explorer_ to copy the contents of the [bin](./bin/) folder (you should copy 4 .bin files) to the _esptool-master_ folder
- Use _Finder_, _Terminal_, or _File Explorer_ to copy the `esp32_arduino_19X_19111.spiffs.bin` file from the _mkspiffs_ folder to the _esptool-master_ folder
- Navigate to the _esptool-master_ folder in _Terminal_ or _Command Prompt_
- Run the command (__don't forget to replace the --port (/dev/cu.wchusbserial1410) option for your T-Stick port__): `esptool.py --chip esp32 --port /dev/cu.wchusbserial1410 --baud 115200 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size detect 0xe000 boot_app0.bin 0x1000 bootloader_dio_80m.bin 0x10000 esp32_arduino_19X_19111.ino.bin 0x8000 esp32_arduino_19X_19111.ino.partitions.bin 2686976 esp32_arduino_19X_19111.spiffs.bin`. Wait for the process to be complete. Do not unplug or turn off your T-Stick during the process.

To test if the data is being send correctly:

- Connect the T-Stick to a network (instructions [here](./Docs/T-Stick_2GW_Connecting_Guide(v1.1).md));
- Open the Pure Data (PD) or Max/MSP patch to receive T-Stick messages (they can be found [here](./Configuration));
- Start receive OSC messages according to the chosen patch.

</blockquote>

### Option 2: Using Arduino IDE

_READ ALL DEPENDENCIES AND OBSERVATIONS BEFORE UPLOAD !_

##### Install Arduino IDE:

To download and install Arduino IDE, follow the instructions at https://www.arduino.cc/en/main/software.

##### Install Arduino ESP32 filesystem uploader: 

You need to upload a file (data/config.json) into ESP32 filesystem. 
Follow the instructions at https://github.com/me-no-dev/arduino-esp32fs-plugin.

##### Install all depencencies:

1. ESP32 Arduino core 1.0.4 or newer. Instructons at https://github.com/espressif/arduino-esp32/blob/master/docs/arduino-ide/boards_manager.md. Install using [boards manager](https://www.arduino.cc/en/guide/cores)
2. For some systems it may be required to also install the ESP8266 Arduino core. Instructons at https://github.com/esp8266/Arduino. Install using [boards manager](https://www.arduino.cc/en/guide/cores)
3. [Wifi32Manager](https://github.com/edumeneses/WiFi32Manager). Instructons at https://github.com/edumeneses/WiFi32Manager#installing. Install using [manual installation](https://www.arduino.cc/en/guide/libraries#toc5)
4. [Adafruit_LSM9DS1](https://github.com/adafruit/Adafruit_LSM9DS1). Install using [library manager](https://www.arduino.cc/en/guide/libraries#toc3)
5. [ArduinoJSON](https://github.com/bblanchon/ArduinoJson) v6.12 or up. Install using [library manager](https://www.arduino.cc/en/guide/libraries#toc3)
6. [CNMAT OSC library](https://github.com/CNMAT/OSC) v3.5.7 or up. Install using [library manager](https://www.arduino.cc/en/guide/libraries#toc3)
7. [DocSunset MIMU Library](https://github.com/DocSunset/MIMU). Install using [manual installation](https://www.arduino.cc/en/guide/libraries#toc5)
8. [Eigen linear algebra library for Arduino](https://github.com/bolderflight/Eigen). Install using [manual installation](https://www.arduino.cc/en/guide/libraries#toc5)

Observations:

1. MINU library complains if you keep any IMU-related files other than MIMU_LSM9DS1.h and MIMU_LSM9DS1.cpp
2. Microcontroller currently in use for T-Stick 2GW: [Wemos LOLIN D32 PRO](https://wiki.wemos.cc/products:d32:d32_pro)

##### Create your custom config.json file:

Each T-Stick uses a _config.json_ file to store all configuration paramethers.

- Make the necessary changes to the [_config.json_](./esp32_arduino_FW200422/data/config.json) file:
  - device: replace _color_ with the shrinking material color, and _19X_ with T-stick's serial number
  - author: replace _IDMIL_ with the builder's name (or alias)
  - nickname: replace _color_ with the shrinking material color
  - id: replace _190_ with your T-Stick serial number
- Save the file at the _data_ folder (inside _esp32_arduino_19X_19101_). Make sure _config.json_ is the only file at _data_ folder

##### Upload (flash) the firmware and config.json into the T-Stick:

- Open the file `esp32_arduino_FW200422.ino` using Arduino IDE
- Choose the proper _board_: `Tools -> Board: "*******" -> LOLIN D32 PRO`
- Choose Upload Speed: `Tools -> Upload Speed: "******" -> 115200`
- Choose port:
  - Keep the T-Stick disconnected to the computer
  - Go to: `Tools -> Port` and take note the available ports
  - connect the T-Stick to the computer using an USB cable and turn the T-Stick ON
  - Go to: `Tools -> Port` and choose the new available port (T-Stick port)
- Upload the firmware: `Sketch -> Upload`. Do not disconnect or turn the T-Stick off during the upload process
- Upload _config.json_: `Tools -> ESP32 Sketch Data Upload`. Do not disconnect or turn the T-Stick off during the upload process

##### Test T-Stick:

To test your T-Stick after flashing firmware/config.json, you can use Arduino IDE:

- Choose T-Stick serial port
- Open the _Serial Monitor_ (`Tools -> Serial Monitor`)

You should see T-Stick booting process.

To test if the data is being send correctly:

- Connect the T-Stick to a network (instructions [here](./Docs/T-Stick_2GW_Connecting_Guide(v1.1).md))
- Open the Pure Data (PD) or Max/MSP patch to receive T-Stick messages (they can be found [here](./Configuration))
- Start receive OSC messages according to the chosen patch

## Update firmware instructions

Updating the firmware does not erase the T-Stick saved configuration and it is a relatively simple process:

:warning: **bin files not available for this firmware version**: Please update using [Arduino IDE](#option-2-using-arduino-ide)

<blockquote>

##### Download the [bin files](./bin):

- Download the .bin files located at the bin [folder](./bin)

##### Download [esptool.py](https://github.com/espressif/esptool):

- Download the _esptool.py_ from https://github.com/espressif/esptool. Use the `Download ZIP` option from Github
- Unzip the _esptool-master.zip_ file

##### Connect the T-Stick to the computer and check the USB port:

- [Check the T-Stick (ESP32) port in your computer](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/establish-serial-connection.html):
  - For MacOS/Linux:
    - Open a _Terminal_ window
    - Execute the command `ls /dev/cu.*`. The command will return a list of ports in your computer.
    - Plug the T-Stick (USB) and run the command `ls /dev/cu.*` one more time. You can now compare the lists and anotate the T-Stick USB port. Should be something similar to `/dev/cu.wchusbserial1410`, probably with a different number
    - Linux users should also give the currently logged user read and write access the serial port over USB. Check [here](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/establish-serial-connection.html) for more information
  - For Windows:
    - Check the list of identified COM ports in the [Windows Device Manager](https://support.microsoft.com/en-ca/help/4026149/windows-open-device-manager)
    - Plug the T-Stick (USB) and check the list of identified COM ports in the [Windows Device Manager](https://support.microsoft.com/en-ca/help/4026149/windows-open-device-manager) again. The T-Stick port should appear on the list. Anotate the T-Stick USB port, it should be something similar to `COM3` or `COM16`


##### Flash the firmware (.bin files):

- Use _Finder_, _Terminal_, or _File Explorer_ to copy the contents of the [bin](./bin/) folder (you should copy 4 .bin files) to the _esptool-master_ folder
- Use _Finder_, _Terminal_, or _File Explorer_ to copy the `esp32_arduino_19X_19111.spiffs.bin` file from the _mkspiffs_ folder to the _esptool-master_ folder
- Navigate to the _esptool-master_ folder in _Terminal_ or _Command Prompt_
- Run the command (__don't forget to replace the --port (/dev/cu.wchusbserial1410) option for your T-Stick port, and the .bin names for the version you plan to upload__): `esptool.py --chip esp32 --port /dev/cu.wchusbserial1410 --baud 115200 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size detect 0xe000 boot_app0.bin 0x1000 bootloader_dio_80m.bin 0x10000 esp32_arduino_19X_19111.ino.bin 0x8000 esp32_arduino_19X_19111.ino.partitions.bin`. Wait for the process to be complete. Do not unplug or turn off your T-Stick during the process.

</blockquote>

##  Other Documentation:

[T-Stick connection guide – v1.1 for wireless T-Sticks](./Docs/T-Stick_2GW_Connecting_Guide(v1.1).md) (model 2GW-19X)

[How to build a T-Stick Sopranino](./Docs/T-Stick_2GW_building_instructions.md)


##  Firmware information:

Sopranino T-Stick 2GW - LOLIN D32 PRO - USB -WiFi  
Input Devices and Music Interaction Laboratory (IDMIL)  
Created: February 2018 by Alex Nieva  
April 2020 by Edu Meneses - firmware version 200422
Notes: Based on test program for reading CY8C201xx using I2C by Joseph Malloch 2011  
Adapted to work with Arduino IDE 1.8.10 and T-Stick Sopranino 2GW  

WiFi32Manager - For use with ESP8266 or ESP32  
Created originally for the T-Stick project:  
http://www-new.idmil.org/project/the-t-stick/  
This code uses code (fork) of 3 other projects:  
https://github.com/kentaylor/WiFiManager    
https://github.com/tzapu/WiFiManager  
https://github.com/zhouhan0126/WIFIMANAGER-ESP32    
Edu Meneses - IDMIL - Mar 2019    

MIMU - Magnetometer + accelerometer + gyroscope orientation library  
https://github.com/DocSunset/MIMU  
Travis West - IDMIL - Oct 2019  
