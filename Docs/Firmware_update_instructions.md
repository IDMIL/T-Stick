# T-Stick 2GW (Firmware version: 220109)

- [T-Stick 2GW (Firmware version: 220109)](#t-stick-2gw-firmware-version-220109)
  - [First time firmware upload instructions](#first-time-firmware-upload-instructions)
    - [Option 1: using .bin files and esptool.py](#option-1-using-bin-files-and-esptoolpy)
      - [Download the bin files](#download-the-bin-files)
      - [Download esptool.py](#download-esptoolpy)
      - [Connect the T-Stick to the computer and check the USB port](#connect-the-t-stick-to-the-computer-and-check-the-usb-port)
      - [Flash the firmware (.bin files)](#flash-the-firmware-bin-files)
    - [Option 2: Using PlatformIO](#option-2-using-platformio)
      - [Install PlatformIO](#install-platformio)
      - [Clone the T-Stick repository](#clone-the-t-stick-repository)
      - [Open firmware project and flash it to the T-Stick](#open-firmware-project-and-flash-it-to-the-t-stick)
      - [Test T-Stick](#test-t-stick)
  - [Other Documentation](#other-documentation)
  - [Firmware information](#firmware-information)

## First time firmware upload instructions

### Option 1: using .bin files and esptool.py

This method is easier/faster. It uses [esptool.py](https://github.com/espressif/esptool).

#### Download the [bin files](../firmware/bin)

- Download the .bin files located at the [bin folder](../firmware/bin)

#### Download [esptool.py](https://github.com/espressif/esptool)

- Download the _esptool.py_ from [https://github.com/espressif/esptool](https://github.com/espressif/esptool). Use the `Download ZIP` option from Github
- Unzip the _esptool-master.zip_ file

#### Connect the T-Stick to the computer and check the USB port

- [Check the T-Stick (ESP32) port in your computer](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/establish-serial-connection.html):
  - For MacOS/Linux:
    - install the [latest drivers from from the SiLabs website](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers).
    - Open a _Terminal_ window
    - Execute the command `ls /dev/cu.*`. The command will return a list of ports in your computer.
    - Plug the T-Stick (USB) and run the command `ls /dev/cu.*` one more time. You can now compare the lists and anotate the T-Stick USB port. Should be something similar to `/dev/cu.wchusbserial1410`, probably with a different number
    - Linux users should also give the currently logged user read and write access the serial port over USB. Check [here](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/establish-serial-connection.html) for more information
  - For Windows:
    - Check the list of identified COM ports in the [Windows Device Manager](https://support.microsoft.com/en-ca/help/4026149/windows-open-device-manager)
    - Plug the T-Stick (USB) and check the list of identified COM ports in the [Windows Device Manager](https://support.microsoft.com/en-ca/help/4026149/windows-open-device-manager) again. The T-Stick port should appear on the list. Anotate the T-Stick USB port, it should be something similar to `COM3` or `COM16`

#### Flash the firmware (.bin files)

- Use _Finder_, _Terminal_, or _File Explorer_ to copy the contents of the [bin](../firmware/bin/) folder (you should copy 3 .bin files) to the _esptool-master_ folder
- Navigate to the _esptool-master_ folder in _Terminal_ or _Command Prompt_
- Run the command (__don't forget to replace the --port (/dev/cu.wchusbserial1410) option for your T-Stick port__): `esptool.py --chip esp32 --port /dev/cu.wchusbserial1410 --baud 115200 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size detect 0xe000 boot_app0.bin 0x1000 bootloader_dio_80m.bin 0x10000 esp32_arduino_FW211124.bin 0x8000 esp32_arduino_FW211124.ino.partitions.bin 2686976 esp32_arduino_FW211124.spiffs.bin`. Wait for the process to be complete. Do not unplug or turn off your T-Stick during the process.

To set the T-Stick info and test if the data is being send correctly:

- Connect the T-Stick to a network (instructions [here](./T-Stick_2GW_Connecting_Guide(v1.2).md));
- Open the Pure Data (PD) or Max/MSP patch to receive T-Stick messages (they can be found [here](../Test_config/));
- Start receive OSC messages according to the chosen patch.

### Option 2: Using PlatformIO

_INSTALL ALL DEPENDENCIES AND REAL ALL OBSERVATIONS BEFORE UPLOAD !_

#### Install PlatformIO

To download and install PlatformIO, follow the instructions at [https://platformio.org/platformio-ide](https://platformio.org/platformio-ide).

We recomment using PlatformIO under Visual Studio Code, but you can also coose another editor.

#### Clone the T-Stick repository

Clone this repository using `git clone https://github.com/IDMIL/T-Stick.git`. Alternatively, you can download the repository as a zip file at [https://github.com/IDMIL/T-Stick](https://github.com/IDMIL/T-Stick). Take note of the folder location.

#### Open firmware project and flash it to the T-Stick

- Open the T-Stick firmware project (folder **firmware** in the T-Stick repository folder) in VSC/PlatformIO. You can get help on how to use PlatformIO at [https://docs.platformio.org/en/latest/core/quickstart.html](https://docs.platformio.org/en/latest/core/quickstart.html)
- You can make any necessary changes on the firmware before flashing (e.g., changing T-Stick ID, selecting the board and capacitive board accordingly)
- If it is the first time flashing, you may see an error pointing to the ESP32 inet.h file. The file requires manual fixing. Check the issue at [https://github.com/mathiasbredholt/libmapper-arduino/issues/3](https://github.com/mathiasbredholt/libmapper-arduino/issues/3)

When ready, you need to flash both the firmware and the filesystem image. Choose the proper platform accordingly (*lolin_d32_pro* or *tinypico*) and use the PlatformIO menu to flash both images to the T-Stick.

#### Test T-Stick

After flashing, you can use the VSC/PlatformIO serial monitor to check if the T-Stick is booting properly. You should see T-Stick booting process.

You can also interact with the controller using the following commands:

- 's' to start setup mode
- 'r' to reboot
- 'd' to enter deep sleep

To test if the data is being send correctly:

- Connect the T-Stick to a network (instructions [here](./Docs/T-Stick_2GW_Connecting_Guide(v1.2).md))
- Open the Pure Data (PD) or Max/MSP patch to receive T-Stick messages (they can be found [here](./Configuration))
- Start receive OSC messages according to the chosen patch

## Other Documentation

[T-Stick connection guide – v1.2 for wireless T-Sticks](./T-Stick_2GW_Connecting_Guide(v1.2).md)

[How to build a T-Stick Sopranino](./T-Stick_2GW_building_instructions.md)

## Firmware information

Sopranino T-Stick 3G - LOLIN D32 PRO / TinyPico - USB - WiFi
Input Devices and Music Interaction Laboratory (IDMIL)  
