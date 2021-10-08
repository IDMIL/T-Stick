# Python code to convert T-Stick serial port messages to OSC
# Author: Edu Meneses (IDMIL, 2019)

import types
import argparse
import sys
import struct
import serial
import collections
from apscheduler.schedulers.background import BackgroundScheduler
from bitstring import BitArray
import netifaces as ni

from oscpy.server import OSCThreadServer
from oscpy.client import OSCClient


# parse argument to set OSC to send/receive T-Stick data
parser = argparse.ArgumentParser(
    description='Convert T-Stick serial port messages into OSC messages.',
    formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument("--serialport", "-s", default="/dev/ttyUSB0",
                    metavar='', help="The T-Stick's serial port")
parser.add_argument("--ip", "-i", default="192.168.0.1",
                    metavar='', help="The ip of the OSC client")
parser.add_argument("--port", "-p", type=int, default=8000,
                    metavar='', help="The port the OSC client is listening on")
parser.add_argument("--networkinterface", "-n", default="wlan0",
                    metavar='', help="The interface connected to the network")
parser.add_argument("--listenport", "-l", type=int, default=8888,
                    metavar='', help="The port this machine is listening on")
args = parser.parse_args()

# set listening ip to receive OSC messages
ni.ifaddresses(args.networkinterface)
LOCAL_IP = ni.ifaddresses(args.networkinterface)[ni.AF_INET][0]['addr']

# Set OSC client and server
SERVER_OSC = OSCThreadServer()
sock = SERVER_OSC.listen(address=LOCAL_IP, port=args.listenport, default=True)
CLIENT_OSC = OSCClient(args.ip, args.port)

# Opening T-Stick serial port
TSTICK = serial.Serial(args.serialport, 115200, dsrdtr=1)

# creating global variable to define T-Stick mapping
TSTICK_ID = 0


def osc_send(sensor_data):
    if sensor_data:
        for i in range(len(sensor_data)):
            if isinstance(sensor_data[i], list):  # has to be list
                data = sensor_data[i]
            if not isinstance(sensor_data[i], list):
                data = [sensor_data[i]]  # adding [] if its not list
            field = sensor_data._fields[i]
            CLIENT_OSC.send_message(b'/{}'.format(field), data)


def resend(*values):
    TSTICK.write(values)


SERVER_OSC.bind(b'/status', resend)


def heartbeat():
    TSTICK.write('s'.encode('utf-8'))


def receive_and_send_serial():
    """ Receive T-Stick serial data and make it available
        as "serial_data" variable. The data is namedtuple.
        Also sends serial data to T-Stick (receives OSC from client).
    """
    flag = False
    msg = []
    while True:
        # byte = int.from_bytes(TSTICK.read(), sys.byteorder)  # python 3.2+
        if sys.byteorder == 'little':
            byte = struct.unpack("<B", TSTICK.read())
        else:
            byte = struct.unpack(">B", TSTICK.read())
        byte = byte[0]
        if not flag:
            if byte == 100:
                information = get_tstick_id(*msg)
                osc_send(information)
                if(TSTICK_ID == 173):
                    serial_data = sort_173(*msg)
                    osc_send(serial_data)
                elif(TSTICK_ID == 10 or
                     TSTICK_ID == 12 or
                     TSTICK_ID == 24 or
                     TSTICK_ID == 171):
                    serial_data = sort_2G(*msg)
                    osc_send(serial_data)
                elif(TSTICK_ID == 15):
                    serial_data = sort_2GX(*msg)
                    osc_send(serial_data)
                del msg[:]
            elif byte == 101:
                flag = True
            else:
                msg.append(byte)
        else:
            msg.append(byte)
            flag = False


def bit_conversion(byte1, byte2):
    return (byte1 * 256) + byte2


def bit_ext_conversion(byte1, byte2):
    bin_or = (byte1 * 256) | byte2
    return bin_or - (65535 * (bin_or > 32767))


def get_tstick_id(*msg):
    named_return = collections.namedtuple(
        'tstick_information', 'tstick_id firmware information')
    if len(msg) < 1:
            return False
    if msg[0] == 0:
        if len(msg) < 5:
            return False
        global TSTICK_ID
        TSTICK_ID = bit_conversion(msg[1], msg[2])
        firmware = bit_conversion(msg[3], msg[4])
        info_list = [TSTICK_ID, firmware]
        if len(msg) > 5:
            for i in msg[5:]:
                info_list.append(i)
        return named_return(TSTICK_ID, firmware, info_list)


def sort_173(*msg):
    """ Route T-Stick messages for T-Stick #173. """
    if msg[0] == 0:
        return False
    elif msg[0] == 1:
        named_return = collections.namedtuple('tstick_sensor', 'rawcapsense')
        rawcapsense = msg[1:]
        return named_return(list(rawcapsense))
    elif msg[0] == 2:
        named_return = collections.namedtuple(
            'tstick_sensor', 'rawaccel rawgyro rawpressure rawpiezo')
        if len(msg) < 17:
            return False
        accel_x = bit_ext_conversion(msg[1], msg[2])
        accel_y = bit_ext_conversion(msg[3], msg[4])
        accel_z = bit_ext_conversion(msg[5], msg[6])
        gyro_x = bit_ext_conversion(msg[7], msg[8])
        gyro_y = bit_ext_conversion(msg[9], msg[10])
        gyro_z = bit_ext_conversion(msg[11], msg[12])
        pressure = bit_ext_conversion(msg[13], msg[14])
        piezo = bit_ext_conversion(msg[15], msg[16])
        return named_return(
            [accel_x, accel_y, accel_z],
            [gyro_x, gyro_y, gyro_z],
            pressure,
            piezo)
    elif msg[0] == 3:
        named_return = collections.namedtuple('tstick_sensor', 'rawmag')
        if len(msg) < 7:
            return False
        mag_x = bit_ext_conversion(msg[1], msg[2])
        mag_y = bit_ext_conversion(msg[3], msg[4])
        mag_z = bit_ext_conversion(msg[5], msg[6])
        return named_return([mag_x, mag_y, mag_z])
    elif msg[0] == 4:
        return False


def sort_2G(*msg):
    """ Route T-Stick messages for T-Stick 2G series: 010, 012, 024, 171. """
    if msg[0] == 0:
        return False
    elif msg[0] == 1:
        named_return = collections.namedtuple('tstick_sensor', 'rawcapsense')
        rawcapsense = msg[1:]
        return named_return(list(rawcapsense))
    elif msg[0] == 2:
        named_return = collections.namedtuple('tstick_sensor', 'rawjab')
        rawjab = msg[1:]
        return named_return(list(rawjab))
    elif msg[0] == 3:
        named_return = collections.namedtuple('tstick_sensor', 'rawtap')
        rawtap = msg[1:]
        return named_return(list(rawtap))
    elif msg[0] == 4:
        named_return = collections.namedtuple(
            'tstick_sensor', 'rawaccel rawpressure rawpiezo')
        if len(msg) < 11:
            return False
        accel_x = bit_conversion(msg[1], msg[2])
        accel_y = bit_conversion(msg[3], msg[4])
        accel_z = bit_conversion(msg[5], msg[6])
        pressure = bit_conversion(msg[7], msg[8])
        piezo = bit_conversion(msg[9], msg[10])
        return named_return([accel_x, accel_y, accel_z], pressure, piezo)


def sort_2GX(*msg):
    """ Route T-Stick messages for T-Stick #015. """
    if msg[0] == 0:
        return False
    elif msg[0] == 1:
        named_return = collections.namedtuple('tstick_sensor', 'rawcapsense')
        rawcapsense = msg[1:]
        return named_return(list(rawcapsense))
        # capsense_bits = cook_touch_soprano(*rawcapsense)
    elif msg[0] == 2:
        named_return = collections.namedtuple('tstick_sensor', 'rawjab')
        rawjab = msg[1:]
        return named_return(list(rawjab))
    elif msg[0] == 3:
        named_return = collections.namedtuple('tstick_sensor', 'rawtap')
        rawtap = msg[1:]
        return named_return(list(rawtap))
    elif msg[0] == 4:
        named_return = collections.namedtuple(
            'tstick_sensor', 'rawaccel rawpressure rawpiezo rawairpressure rawrange rawldr1 rawldr2')
        if len(msg) < 19:
            return False
        accel_x = bit_conversion(msg[1], msg[2])
        accel_y = bit_conversion(msg[3], msg[4])
        accel_z = bit_conversion(msg[5], msg[6])
        pressure = bit_conversion(msg[7], msg[8])
        piezo = bit_conversion(msg[9], msg[10])
        airpressure = bit_conversion(msg[11], msg[12])
        tstick_range = bit_conversion(msg[13], msg[14])
        ldr1 = bit_conversion(msg[15], msg[16])
        ldr2 = bit_conversion(msg[17], msg[18])
        return named_return([accel_x, accel_y, accel_z], pressure, piezo, airpressure, tstick_range, ldr1, ldr2)


def cook_touch_soprano(*bytes):
    byte_list = ""
    for byte in bytes:
        byte_list = byte_list + format(byte, '08b')
    return list(byte_list)


def tstick_wakeup():
    """ Setting heartbeat to run every second """
    scheduler = BackgroundScheduler()
    scheduler.add_job(heartbeat, 'interval', seconds=1)
    scheduler.start()


if __name__ == '__main__':

    tstick_wakeup()

    receive_and_send_serial()

    sys.exit()
