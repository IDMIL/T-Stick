# T-Stick Conversion Box - Building recipe - Rpi 2B and 3B+

Under Raspbian Lite

## Prepare the OS

- Flash the latest Prynth image (https://prynth.github.io/create/downloads.html)
- Add ssh file to Boot partition: `touch ssh`
- Add wpa_supplicant.conf to file to Boot partition: `vim wpa_supplicant.conf`. File contents:

```bash
ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev
update_config=1
country=US

network={
     ssid="Your network name/SSID"
     psk="Your WPA/WPA2 security key"
     key_mgmt=WPA-PSK
}
```

- Open Prynth software settings at `http://raspberrypi.local:3000/system` and change hostname to match the converted T-Stick  
- SSH, change password, hostname, set country, expand file system (raspi-config)

## Update

```bash
sudo apt-get -y update
sudo apt-get -y dist-upgrade
```

## Install liblo

```bash
sudo apt-get remove -y liblo-dev
mkdir ~/sources
cd ~/sources
git clone https://github.com/radarsat1/liblo.git
cd liblo
sudo apt-get install -y vim autoconf libtool automake
./autogen.sh
make
sudo make install
```

## Install Libmapper

```bash
sudo apt-get install python-dev swig
cd ~/sources
git clone https://github.com/libmapper/libmapper.git
cd libmapper
./autogen.sh
echo 'export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib' >> ~/.bashrc
make
sudo make install
```

## Install Webmapper

```bash
pip install netifaces
cd ~/sources
git clone https://github.com/libmapper/webmapper.git
cp ~/sources/libmapper/swig/_mapper.so ~/sources/webmapper
cp ~/sources/libmapper/swig/mapper.py ~/sources/webmapper
cd webmapper/
```

- To run Webmapper: ./webmapper.py

## Install Python 3.7.3

```bash
sudo apt-get install -y build-essential tk-dev libncurses5-dev libncursesw5-dev libreadline6-dev libdb5.3-dev libgdbm-dev libsqlite3-dev libssl-dev libbz2-dev libexpat1-dev liblzma-dev zlib1g-dev libffi-dev apt-show-versions
cd ~/sources
wget https://www.python.org/ftp/python/3.7.3/Python-3.7.3.tgz
tar -zxvf Python-3.7.3.tgz
cd Python-3.7.3/
./configure --enable-optimizations
make -j4
make test
sudo make install
```

Reboot


## Create python application to read serial port and send OSC messages

- Install packages:

```bash
cd ~
sudo apt-get install -y python3-pip python-pip setserial
sudo pip3 install python-osc pyserial apscheduler bitstring netifaces
pip install pyserial apscheduler bitstring pyOSC netifaces oscpy
mkdir tstick
```

- Create code:

```bash
cat <<- "EOF" | sudo tee ~/tstick/tstick.py
# Python code to convert T-Stick serial port messages to OSC
# Author: Edu Meneses (IDMIL, 2019)

import sys
import serial
import collections
from apscheduler.schedulers.background import BackgroundScheduler
from bitstring import BitArray
import netifaces as ni

import argparse
from pythonosc import osc_message_builder
from pythonosc import udp_client
from pythonosc.dispatcher import Dispatcher

from pythonosc.osc_server import AsyncIOOSCUDPServer
import asyncio


# parse argument to set OSC to send/receive T-Stick data
parser = argparse.ArgumentParser(
    description='Convert T-Stick serial port messages into OSC messages.',
    formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument("--serialport", "-s", default="/dev/ttyUSB0",
                    metavar='', help="The T-Stick's serial port")
parser.add_argument("--ip", "-i", default="192.168.0.1",
                    metavar='', help="The ip of the OSC server")
parser.add_argument("--port", "-p", type=int, default=8000,
                    metavar='', help="The port the OSC server is listening on")
parser.add_argument("--networkinterface", "-n", default="wlan0",
                    metavar='', help="The interface connected to the network")
parser.add_argument("--listenport", "-l", type=int, default=8888,
                    metavar='', help="The port this machine is listening on")
args = parser.parse_args()

# set listening ip to receive OSC messages
ni.ifaddresses(args.networkinterface)
local_ip = ni.ifaddresses(args.networkinterface)[ni.AF_INET][0]['addr']

# Opening T-Stick serial port
tstick = serial.Serial(args.serialport, 115200, dsrdtr=1)

# checking system byteorder
byteorder = sys.byteorder

# creating global variable to define T-Stick mapping
tstick_id = 0

dispatcher = Dispatcher()

client_OSC = udp_client.SimpleUDPClient(args.ip, args.port)


def resend(address, *args):
    tstick.write(args)


def heartbeat():
    tstick.write('s'.encode('utf-8'))


async def receive_serial():
    flag = False
    msg = []
    while True:
        byte = int.from_bytes(tstick.read(), byteorder)
        if not flag:
            if byte == 100:
                get_tstick_id(*msg)
                if(tstick_id == 173):
                    sort_and_send_173(*msg)
                elif(tstick_id == 10 or
                     tstick_id == 12 or
                     tstick_id == 24 or
                     tstick_id == 171):
                    sort_and_send_2G(*msg)
                elif(tstick_id == 15):
                    sort_and_send_2GX(*msg)
                msg.clear()
            elif byte == 101:
                flag = True
            else:
                msg.append(byte)
        else:
            msg.append(byte)
            flag = False
        await asyncio.sleep(0)


# 'convert' 10 bits
def bit_conversion(byte1, byte2):
    return (byte1 * 256) + byte2


# T-stick 16 bit conversion IMU
def bit_ext_conversion(byte1, byte2):
    bin_or = (byte1 * 256) | byte2
    return bin_or - (65535 * (bin_or > 32767))


def get_tstick_id(*msg):
    if msg[0] == 0:
        if len(msg) < 3:
            return 0
        global tstick_id
        tstick_id = bit_conversion(msg[1], msg[2])
        firmware = bit_conversion(msg[3], msg[4])
        info_list = [tstick_id, firmware]
        if len(msg) > 5:
            for i in msg[5:]:
                info_list.append(i)
        client_OSC.send_message("/information", info_list)
        return tstick_id


# route T-Stick messages for T-Stick #173
def sort_and_send_173(*msg):
    if msg[0] == 0:
        pass
    elif msg[0] == 1:
        rawcapsense = msg[1:]
        client_OSC.send_message("/rawcapsense", rawcapsense)
        # capsense_bits = cook_touch_soprano(*rawcapsense)
    elif msg[0] == 2:
        accel_x = bit_ext_conversion(msg[1], msg[2])
        accel_y = bit_ext_conversion(msg[3], msg[4])
        accel_z = bit_ext_conversion(msg[5], msg[6])
        gyro_x = bit_ext_conversion(msg[7], msg[8])
        gyro_y = bit_ext_conversion(msg[9], msg[10])
        gyro_z = bit_ext_conversion(msg[11], msg[12])
        pressure = bit_ext_conversion(msg[13], msg[14])
        piezo = bit_ext_conversion(msg[15], msg[16])
        client_OSC.send_message("/rawaccel", [accel_x, accel_y, accel_z])
        client_OSC.send_message("/rawgyro", [gyro_x, gyro_y, gyro_z])
        client_OSC.send_message("/rawpressure", pressure)
        client_OSC.send_message("/rawpiezo", piezo)
    elif msg[0] == 3:
        mag_x = bit_ext_conversion(msg[1], msg[2])
        mag_y = bit_ext_conversion(msg[3], msg[4])
        mag_z = bit_ext_conversion(msg[5], msg[6])
        client_OSC.send_message("/rawmag", [mag_x, mag_y, mag_z])
    elif msg[0] == 4:
        return 0


# route T-Stick messages for T-Stick 2G series: 010, 012, 024, 171
def sort_and_send_2G(*msg):
    if msg[0] == 0:
        pass
    elif msg[0] == 1:
        rawcapsense = msg[1:]
        client_OSC.send_message("/rawcapsense", rawcapsense)
        # capsense_bits = cook_touch_soprano(*rawcapsense)
    elif msg[0] == 2:
        rawjab = msg[1:]
        client_OSC.send_message("/rawjab", rawjab)
    elif msg[0] == 3:
        rawtap = msg[1:]
        client_OSC.send_message("/rawtap", rawtap)
    elif msg[0] == 4:
        accel_x = bit_conversion(msg[1], msg[2])
        accel_y = bit_conversion(msg[3], msg[4])
        accel_z = bit_conversion(msg[5], msg[6])
        pressure = bit_conversion(msg[7], msg[8])
        piezo = bit_conversion(msg[9], msg[10])
        client_OSC.send_message("/rawaccel", [accel_x, accel_y, accel_z])
        client_OSC.send_message("/rawpressure", pressure)
        client_OSC.send_message("/rawpiezo", piezo)


# route T-Stick messages for T-Stick #015
def sort_and_send_2GX(*msg):
    if msg[0] == 0:
        pass
    elif msg[0] == 1:
        rawcapsense = msg[1:]
        client_OSC.send_message("/rawcapsense", rawcapsense)
        # capsense_bits = cook_touch_soprano(*rawcapsense)
    elif msg[0] == 2:
        rawjab = msg[1:]
        client_OSC.send_message("/rawjab", rawjab)
    elif msg[0] == 3:
        rawtap = msg[1:]
        client_OSC.send_message("/rawtap", rawtap)
    elif msg[0] == 4:
        accel_x = bit_conversion(msg[1], msg[2])
        accel_y = bit_conversion(msg[3], msg[4])
        accel_z = bit_conversion(msg[5], msg[6])
        pressure = bit_conversion(msg[7], msg[8])
        piezo = bit_conversion(msg[9], msg[10])
        airpressure = bit_conversion(msg[11], msg[12])
        tstick_range = bit_conversion(msg[13], msg[14])
        ldr1 = bit_conversion(msg[15], msg[16])
        ldr2 = bit_conversion(msg[17], msg[18])
        client_OSC.send_message("/rawaccel", [accel_x, accel_y, accel_z])
        client_OSC.send_message("/rawpressure", pressure)
        client_OSC.send_message("/rawpiezo", piezo)
        client_OSC.send_message("/rawairpressure", airpressure)
        client_OSC.send_message("/rawrange", tstick_range)
        client_OSC.send_message("/rawldr1", ldr1)
        client_OSC.send_message("/rawldr2", ldr2)


# make bit list - 1 per cap stripe
def cook_touch_soprano(*bytes):
    byte_list = ""
    for byte in bytes:
        byte_list = byte_list + format(byte, '08b')
    return list(byte_list)


# setting heartbeat to run every second
def tstick_wakeup():
    scheduler = BackgroundScheduler()
    scheduler.add_job(heartbeat, 'interval', seconds=1)
    scheduler.start()


async def init_main():
    # Create datagram endpoint and start serving
    server_OSC = AsyncIOOSCUDPServer(
        (local_ip, args.listenport), dispatcher, asyncio.get_event_loop())

    transport, protocol = await server_OSC.create_serve_endpoint()

    dispatcher.map("/status", resend)

    tstick_wakeup()

    await receive_serial()

    transport.close()  # Clean up serve endpoint

    sys.exit()


asyncio.run(init_main())  # Init and enter main loop of program

EOF

```
Set the code to load at boot:

```bash
cat <<- "EOF" | sudo tee /lib/systemd/system/tstick.service
[Unit]
Description=T-Stick serial port to OSC conversion

[Service]
ExecStart=/usr/bin/python3 /home/pi/tstick/tstick.py
#User=pi

[Install]
WantedBy=multi-user.target
EOF

```

```bash
sudo systemctl daemon-reload &&\
sudo systemctl enable hardwarecontrol.service
```

Old, unnecessary packages are removed after the system update with the following commands:

```bash
sudo apt-get autoremove
sudo apt-get autoclean
```