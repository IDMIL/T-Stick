# Python code to convert T-Stick serial port messages to OSC
# Author: Edu Meneses (IDMIL, 2019)

import sys
import serial
import collections
import struct
from apscheduler.schedulers.background import BackgroundScheduler
from bitstring import BitArray

import argparse


# parse argument to set OSC to send/receive T-Stick data
parser = argparse.ArgumentParser(
    description='Convert T-Stick serial port messages into OSC messages.',
    formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument("--serialport", "-s", default="/dev/ttyUSB0",
                    metavar='', help="The T-Stick's serial port")
args = parser.parse_args()

# Opening T-Stick serial port
tstick = serial.Serial(args.serialport, 115200, dsrdtr=1)

# checking system byteorder
byteorder = sys.byteorder

# creating global variable to define T-Stick mapping
tstick_id = 0


def resend(address, *args):
    tstick.write(args)


def heartbeat():
    tstick.write('s'.encode('utf-8'))


def receive_serial():
    """ Receive T-Stick serial data and make it available
        as "serial_data" variable. The data is namedtuple.
    """
    flag = False
    msg = []
    while True:
        # byte = int.from_bytes(tstick.read(), sys.byteorder)  # python 3.2+
        byte = tstick.read()
        if sys.byteorder is 'little':
            byte = struct.unpack("<B", tstick.read())
        else:
            byte = struct.unpack(">B", tstick.read())
        if not flag:
            if byte == 100:
                information = get_tstick_id(*msg)
                if(tstick_id == 173):
                    serial_data = sort_and_send_173(*msg)
                elif(tstick_id == 10 or
                     tstick_id == 12 or
                     tstick_id == 24 or
                     tstick_id == 171):
                    serial_data = sort_and_send_2G(*msg)
                elif(tstick_id == 15):
                    serial_data = sort_and_send_2GX(*msg)
                msg.clear()
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
        'tstick_information', 'tstick_id firmware info_list')
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
        return named_return(tstick_id, firmware, info_list)


def sort_and_send_173(*msg):
    """ Route T-Stick messages for T-Stick #173. """
    if msg[0] == 0:
        return 0
    elif msg[0] == 1:
        named_return = collections.namedtuple('tstick_sensor', 'rawcapsense')
        rawcapsense = msg[1:]
        return named_return(rawcapsense)
    elif msg[0] == 2:
        named_return = collections.namedtuple(
            'tstick_sensor', 'rawaccel rawgyro rawpressure rawpiezo')
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
        mag_x = bit_ext_conversion(msg[1], msg[2])
        mag_y = bit_ext_conversion(msg[3], msg[4])
        mag_z = bit_ext_conversion(msg[5], msg[6])
        return named_return([mag_x, mag_y, mag_z])
    elif msg[0] == 4:
        return 0


def sort_and_send_2G(*msg):
    """ Route T-Stick messages for T-Stick 2G series: 010, 012, 024, 171. """
    if msg[0] == 0:
        return 0
    elif msg[0] == 1:
        named_return = collections.namedtuple('tstick_sensor', 'rawcapsense')
        rawcapsense = msg[1:]
        return named_return(rawcapsense)
    elif msg[0] == 2:
        named_return = collections.namedtuple('tstick_sensor', 'rawjab')
        rawjab = msg[1:]
        return named_return(rawjab)
    elif msg[0] == 3:
        named_return = collections.namedtuple('tstick_sensor', 'rawtap')
        rawtap = msg[1:]
        return named_return(rawtap)
    elif msg[0] == 4:
        named_return = collections.namedtuple(
            'tstick_sensor', 'rawaccel rawpressure rawpiezo')
        accel_x = bit_conversion(msg[1], msg[2])
        accel_y = bit_conversion(msg[3], msg[4])
        accel_z = bit_conversion(msg[5], msg[6])
        pressure = bit_conversion(msg[7], msg[8])
        piezo = bit_conversion(msg[9], msg[10])
        return named_return([accel_x, accel_y, accel_z], pressure, piezo)


def sort_and_send_2GX(*msg):
    """ Route T-Stick messages for T-Stick #015. """
    if msg[0] == 0:
        return 0
    elif msg[0] == 1:
        named_return = collections.namedtuple('tstick_sensor', 'rawcapsense')
        rawcapsense = msg[1:]
        return named_return(rawcapsense)
        # capsense_bits = cook_touch_soprano(*rawcapsense)
    elif msg[0] == 2:
        named_return = collections.namedtuple('tstick_sensor', 'rawjab')
        rawjab = msg[1:]
        return named_return(rawjab)
    elif msg[0] == 3:
        named_return = collections.namedtuple('tstick_sensor', 'rawtap')
        rawtap = msg[1:]
        return named_return(rawtap)
    elif msg[0] == 4:
        named_return = collections.namedtuple(
            'tstick_sensor', 'rawaccel rawpressure rawpiezo rawairpressure rawrange rawldr1 rawldr2')
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

    receive_serial()

    sys.exit()
