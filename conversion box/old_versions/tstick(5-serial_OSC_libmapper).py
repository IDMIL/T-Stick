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

import mapper

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


class tstick:

    # Creating variables to define T-Stick mapping and
    # to create a libmapper device.
    # Those variables will be defined when we receive
    # T-Stick info.
    tstick_id = 0
    libmapper_configured = False
    libmapper_signals = {}

    def __init__(self, serialport, ip, port, networkinterface, listenport):

        # set listening ip to receive OSC messages
        self.local_ip = ni.ifaddresses(args.networkinterface)[
            ni.AF_INET][0]['addr']

        # Set OSC client and server
        self.server_osc = OSCThreadServer()
        self.sock = self.server_osc.listen(
            address=self.local_ip, port=listenport, default=True)
        self.client_osc = OSCClient(ip, port)

        # Opening T-Stick serial port
        self.tstick_serial_port = serial.Serial(serialport, 115200, dsrdtr=1)

        # routing OSC signals
        self.server_osc.bind(b'/status', self.resend)

    def libmapper_setup(self, id_number):
        self.libmapper_dev = mapper.device("T-Stick{0:03d}".format(id_number))
        self.libmapper_configured = True

    def osc_send(self, sensor_data):
        if sensor_data:
            for i in range(len(sensor_data)):
                if isinstance(sensor_data[i], list):  # has to be list
                    data = data_lib = sensor_data[i]
                if not isinstance(sensor_data[i], list):
                    data_lib = sensor_data[i]
                    data = [sensor_data[i]]  # adding [] if its not list
                sensor_name = sensor_data._fields[i]
                if sensor_name not in self.libmapper_signals:
                    self.libmapper_signals[sensor_name] = self.libmapper_dev.add_output_signal(
                        "{}".format(sensor_name), len(data), 'i')
                self.client_osc.send_message(b'/{}'.format(sensor_name), data)
                # updating libmapper signals dynamically
                self.libmapper_signals[sensor_name].update(data_lib)

    def resend(self, *values):
        self.tstick_serial_port.write(values)

    def heartbeat(self):
        self.tstick_serial_port.write('s'.encode('utf-8'))

    def receive_and_send_serial(self):
        """ Receive T-Stick serial data and make it available
            as "serial_data" variable. The data is namedtuple.
            Also sends serial data to T-Stick (receives OSC from client).
        """
        flag = False
        msg = []
        while True:
            # byte = int.from_bytes(TSTICK.read(), sys.byteorder)  # python 3.2+
            if sys.byteorder == 'little':
                byte = struct.unpack("<B", self.tstick_serial_port.read())
            else:
                byte = struct.unpack(">B", self.tstick_serial_port.read())
            byte = byte[0]
            if not flag:
                if byte == 100:
                    information = self.get_tstick_id(*msg)
                    self.osc_send(information)
                    if(self.tstick_id == 173):
                        serial_data = self.sort_173(*msg)
                        self.osc_send(serial_data)
                    elif(self.tstick_id == 10 or
                         self.tstick_id == 12 or
                         self.tstick_id == 24 or
                         self.tstick_id == 171):
                        serial_data = self.sort_2G(*msg)
                        self.osc_send(serial_data)
                    elif(self.tstick_id == 15):
                        serial_data = self.sort_2GX(*msg)
                        self.osc_send(serial_data)
                    del msg[:]
                elif byte == 101:
                    flag = True
                else:
                    msg.append(byte)
            else:
                msg.append(byte)
                flag = False
            # if self.libmapper_configured:
               # self.libmapper_dev.poll(0)  # libmapper polling

    def bit_conversion(self, byte1, byte2):
        return (byte1 * 256) + byte2

    def bit_ext_conversion(self, byte1, byte2):
        bin_or = (byte1 * 256) | byte2
        return bin_or - (65535 * (bin_or > 32767))

    def get_tstick_id(self, *msg):
        named_return = collections.namedtuple(
            'tstick_information', 'information')
        if len(msg) < 1:
            return False
        if msg[0] == 0:
            if len(msg) < 5:
                return False
            self.tstick_id = self.bit_conversion(msg[1], msg[2])
            firmware = self.bit_conversion(msg[3], msg[4])
            info_list = [self.tstick_id, firmware]
            if len(msg) > 5:
                for i in msg[5:]:
                    info_list.append(i)
            if self.libmapper_configured is False:
                self.libmapper_setup(self.tstick_id)
            return named_return(info_list)

    def sort_173(self, *msg):
        """ Route T-Stick messages for T-Stick #173. """
        if msg[0] == 0:
            return False
        elif msg[0] == 1:
            named_return = collections.namedtuple(
                'tstick_sensor', 'rawcapsense')
            rawcapsense = msg[1:]
            return named_return(list(rawcapsense))
        elif msg[0] == 2:
            named_return = collections.namedtuple(
                'tstick_sensor', 'rawaccel rawgyro rawpressure rawpiezo')
            if len(msg) < 17:
                return False
            accel_x = self.bit_ext_conversion(msg[1], msg[2])
            accel_y = self.bit_ext_conversion(msg[3], msg[4])
            accel_z = self.bit_ext_conversion(msg[5], msg[6])
            gyro_x = self.bit_ext_conversion(msg[7], msg[8])
            gyro_y = self.bit_ext_conversion(msg[9], msg[10])
            gyro_z = self.bit_ext_conversion(msg[11], msg[12])
            pressure = self.bit_ext_conversion(msg[13], msg[14])
            piezo = self.bit_ext_conversion(msg[15], msg[16])
            return named_return(
                [accel_x, accel_y, accel_z],
                [gyro_x, gyro_y, gyro_z],
                pressure,
                piezo)
        elif msg[0] == 3:
            named_return = collections.namedtuple('tstick_sensor', 'rawmag')
            if len(msg) < 7:
                return False
            mag_x = self.bit_ext_conversion(msg[1], msg[2])
            mag_y = self.bit_ext_conversion(msg[3], msg[4])
            mag_z = self.bit_ext_conversion(msg[5], msg[6])
            return named_return([mag_x, mag_y, mag_z])
        elif msg[0] == 4:
            return False

    def sort_2G(self, *msg):
        """ Route T-Stick messages for T-Stick 2G series: 010, 012, 024, 171. """
        if msg[0] == 0:
            return False
        elif msg[0] == 1:
            named_return = collections.namedtuple(
                'tstick_sensor', 'rawcapsense')
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
            accel_x = self.bit_conversion(msg[1], msg[2])
            accel_y = self.bit_conversion(msg[3], msg[4])
            accel_z = self.bit_conversion(msg[5], msg[6])
            pressure = self.bit_conversion(msg[7], msg[8])
            piezo = self.bit_conversion(msg[9], msg[10])
            return named_return([accel_x, accel_y, accel_z], pressure, piezo)

    def sort_2GX(*msg):
        """ Route T-Stick messages for T-Stick #015. """
        if msg[0] == 0:
            return False
        elif msg[0] == 1:
            named_return = collections.namedtuple(
                'tstick_sensor', 'rawcapsense')
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
            accel_x = self.bit_conversion(msg[1], msg[2])
            accel_y = self.bit_conversion(msg[3], msg[4])
            accel_z = self.bit_conversion(msg[5], msg[6])
            pressure = self.bit_conversion(msg[7], msg[8])
            piezo = self.bit_conversion(msg[9], msg[10])
            airpressure = self.bit_conversion(msg[11], msg[12])
            tstick_range = self.bit_conversion(msg[13], msg[14])
            ldr1 = self.bit_conversion(msg[15], msg[16])
            ldr2 = self.bit_conversion(msg[17], msg[18])
            return named_return([accel_x, accel_y, accel_z], pressure, piezo, airpressure, tstick_range, ldr1, ldr2)

    def cook_touch_soprano(self, *bytes):
        byte_list = ""
        for byte in bytes:
            byte_list = byte_list + format(byte, '08b')
        return list(byte_list)

    def wakeup(self):
        """ Setting heartbeat to run every second """
        scheduler = BackgroundScheduler()
        scheduler.add_job(self.heartbeat, 'interval', seconds=1)
        scheduler.start()


if __name__ == '__main__':

    tstick = tstick(args.serialport, args.ip, args.port,
                    args.networkinterface, args.listenport)

    tstick.wakeup()

    tstick.receive_and_send_serial()

    sys.exit()
