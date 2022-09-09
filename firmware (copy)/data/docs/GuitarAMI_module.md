# GuitarAMI Module user guide

![Modules](./images_module/modules.jpg "Modules")

- [Usage instructions](#usage-instructions)
  - [Operation modes](#operation-modes)
  - [Setup](#setup)
  - [LED indicator](#led-indicator)
  - [Charging your module](#charging-your-module)
- [OSC namespaces](#osc-namespaces)
- [MIDI messages](#midi-messages)
- [Pure Data test patch](#pure-data-test-patch)
- [Updating module firmware](#updating-module-firmware)
- [Other Documentation](#other-documentation)
- [Information](#information)

## Usage instructions

### Operation modes

The module can operate in 3 different modes:

- OSC-STA mode: The module becomes a WiFi client (station mode) and will send OSC messages to the addresses/ports configured on the setup page. Both the module and recipient must be connected to the same network.
- OSC-AP mode: The module creates it's own WiFi network (access point) and will send OSC messages to the addresses/ports configured on the setup. The AP created uses credentials configured on the setup page and recipient must be connected to the module's network. The AP's name is created using the instrument's name, e.g, *GuitarAMI_module_XXX*. The default password is `mappings`.
- MIDI mode: The module behaves as a BLE MIDI device, and can be used as a wireless midi controller.

### Setup

The module also has a special setup mode. To access the mode, hold the capacitive touch for 5 seconds. The module will reboot and enter the setup mode. You can access the setup page using 2 different methods:

- Using the module's AP:
  - Access the AP (created using the instrument's name, e.g, *GuitarAMI_module_XXX* and previously stored password)
  - In a new browser window, type either [http://GuitarAMI_module_XXX.local/](http://GuitarAMI_module_XXX.local/) (replace the XXX by your module's ID) or the IP address (if known), e.g., *192.168.0.100*
- Using the current network (STA):
  - In a new browser window, type either [http://GuitarAMI_module_XXX.local/](http://GuitarAMI_module_XXX.local/) (replace the XXX by your module's ID) or the IP address (if known), e.g., *192.168.0.100*

The setup options include:

- Network:
  - SSID: network SSID for STA mode (will auto-fill with current SSID)
  - SSID Password: password for STA mode network (will auto-fill with current password)
- GuitarAMI module password:
  - New password: type password for AP and setup modes (leve it blank if you want keep the same password)
  - Retype new password: retype password for AP and setup modes (leve it blank if you want keep the same password)
- Operation mode:
  - STA (station), AP (access point), MIDI
- OSC send settings:
  - Primary IP: first IP address to send OSC messages (STA and AP modes)
  - Primary port: first port to send OSC messages (STA and AP modes)
  - Secondary IP: second IP address to send OSC messages (STA and AP modes)
  - Secondary port: second port to send OSC messages (STA and AP modes)
- Scan for networks: open a secondary page with the scanned networks in the area. This list may not be complete and it's necessary to fill the SSID (for STA mode) manually on the setup page (you can copy/paste the SSID from the scan page)
- SAVE button: click to save the surrent configuration

The setup page also contains extra information about the module: Current SSID, Current IP address (STA), Access Point IP, Station (STA) MAC address, Access Point MAC address, module ID (serial #), module author, Institution, and Firmware version.

To exit the setup mode, click on `Close and Reboot`. The module will reboot into the chosen operation mode.

### LED indicator

The GuitarAMI module has a built-in LED to indicate status and battery level.

- For the modules using the TinyPICO (with RGB LED):
  - Low battery indicator: fast blink (flickering) in Red
  - Mode indicator (color):
    - STA: Dodger Blue
    - AP: Lime
    - MIDI: Magenta
    - Setup: Dark Orange
  - Connection indicator: the LED will blink slowly when connected, while it will cycle-fade when disconnected
  - Battery Charge: Orange (on secondary LED close to USB port)
  - USB Power connection: Red (on secondary LED close to USB port)
  - Error: if everything goes wrong it cycles through all colors (rainbow panic)

- For the older modules using the Wemos D32 Pro (with blue LED next to the USB port):
  - Low battery indicator: fast blink (flickering)
  - Setup mode: the LED will be always ON
  - Connection indicator: the LED will blink slowly when connected, while it will cycle-fade when disconnected

### Charging your module

To charge the module, connect any 5V USB power supply to the micro-USB port. A red light will indicate that the module is charging.

IMPORTANT: The module needs to be turned ON to charge!
{: .alert .alert-warning}

## OSC namespaces

- continuous:
  - /GuitarAMI_module_XXX/ult [int] (in mm)
  - /GuitarAMI_module_XXX/accl [float] [float] [float] (in m/s^2)
  - /GuitarAMI_module_XXX/gyro [float] [float] [float] (in radians per second)
  - /GuitarAMI_module_XXX/magn [float] [float] [float] (in uTesla)
  - /GuitarAMI_module_XXX/quat [float] [float] [float]
- discrete:
  - /GuitarAMI_module_XXX/tap [0 or 1]
  - /GuitarAMI_module_XXX/dtap [0 or 1]
  - /GuitarAMI_module_XXX/ttap [0 or 1]
  - /GuitarAMI_module_XXX/battery [int] (percentage)
- Instrument:
  - /GuitarAMI_module_XXX/ypr [float] [float] [float] (in degrees)
  - /GuitarAMI_module_XXX/jab [float] [float] [float]
  - /GuitarAMI_module_XXX/shake [float] [float] [float]
  - /GuitarAMI_module_XXX/count [int]
  - /GuitarAMI_module_XXX/ultTrig [int] [0 or 1]

## MIDI messages

- continuous:
  - CC 20: ultrasonic sensor scaled from 0 to 300 mm
  - CC 21: absolute X-axis acceleration, scaled from 0 to 50 m/s^2 (~5.1 gs)
  - CC 22: absolute Y-axis acceleration, scaled from 0 to 50 m/s^2 (~5.1 gs)
  - CC 23: absolute Z-axis acceleration, scaled from 0 to 50 m/s^2 (~5.1 gs)
  - CC 24: absolute X-axis angular velocity (gyroscope), scaled from 0 to 25 rad/s
  - CC 25: absolute Y-axis angular velocity (gyroscope), scaled from 0 to 25 rad/s
  - CC 26: absolute Z-axis angular velocity (gyroscope), scaled from 0 to 25 rad/s
  - CC 27: yaw, scaled from -180 to 180 degrees
  - CC 28: pitch, scaled from -180 to 180 degrees
  - CC 29: roll, scaled from -180 to 180 degrees
  - CC 85: shake X
  - CC 86: shake Y
  - CC 87: shake Z
  - CC 102: jab X
  - CC 103: jab Y
  - CC 104: jab Z
- discrete:
  - CC 30: tap [0 or 127]
  - CC 31: double tap [0 or 127]
  - CC 14: triple tap [0 or 127]
  - CC 15: touch count starting from 0
  - CC 89: Ultrasonic trigger [0 or 127]

## Pure Data test patch

There is an GuitarAMI module [Pure Data](http://puredata.info/) test patch for OSC and MIDI that can be downloaded [directly from the module](/GuitarAMI_module.pd) ([GuitarAMI_module.pd](/GuitarAMI_module.pd)). Users can check if all data is reaching the destination and the interval between messages.

## Updating module firmware

To update your firmware, please follow the following steps:
  
- Download the new firmware or spiffs file (.bin) provided by the module's author
- Enter the [Setup mode](#setup)
- In a new browser window, type either [http://GuitarAMI_module_XXX.local/update](http://GuitarAMI_module_XXX.local/update) (replace the XXX by your module's ID) or the IP address (if known), e.g., [192.168.0.100/update](192.168.0.100/update)
- Click on `choose file` and select the apropriate file. Check if the filename is correct (next to the button)
- Click on `Update Firmware` or `Update SPIFFS` according to the chosen file
- **DO NOT turn the module OFF during the process**. Wait until the module reboot itself: the update page will be unavailable and the module will enter setup mode again. You can then access the [setup page](#setup) and click on `Close and reboot`

## Information

GuitarAMI (classical guitar) Module - WiFi(OSC) + MIDI
[Input Devices and Music Interaction Laboratory (IDMIL)](http://www.idmil.org/)
[Centre of Interdisciplinary Research In Music Media and Technology (CIRMMT)](http://www.cirmmt.org/)
February 2021 by [Edu Meneses](http://www.edumeneses.com/)
