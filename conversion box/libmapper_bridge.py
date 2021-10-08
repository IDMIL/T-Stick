# Python code to autoconfig libmapper devices using OSC messages
# Author: Edu Meneses (IDMIL, 2019)

#import types
import argparse
#import sys
#import struct
import serial
import collections
#from bitstring import BitArray
#import netifaces as ni


import threading
import mapper

import time

# parse argument to set OSC to send/receive T-Stick data
parser = argparse.ArgumentParser(
    description='Libmapper auto configuration tool.',
    formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument("--name", "-n", default="instrument",
                    metavar='', help="The instrument's name")
args = parser.parse_args()


class libmapper_bridge:

    # Creating signal dictionary to dynamically create
    # libmapper signals.
    libmapper_signal = {}
    libmapper_signal_length = {}

    def __init__(self, instrument_name):

        self.libmapper_dev = mapper.device(instrument_name)

    def signal_update(self, sensor_name, sensor_data):
        if sensor_data:
            if sensor_name not in self.libmapper_signals:
                self.libmapper_signal[sensor_name] = self.libmapper_dev.add_output_signal(
                    "{}".format(sensor_name), len(sensor_data), 'i')
                self.libmapper_signal_length[sensor_name] = len(sensor_data)
                return 0
            # updating libmapper signals dynamically
            self.libmapper_signal[sensor_name].update(sensor_data)
            return 1

    def poller(self, argumento):
        self.libmapper_dev.poll(50)  # libmapper polling
        print("polled")


if __name__ == '__main__':

    libmapper_bridge = libmapper_bridge("teste")

    # libmapper_poller = threading.Thread(
    #   target=fakeinstrument.poller)  # , daemon=True)

    # fakeinstrument.libmapper_signal['dummy'] =

    dummy = libmapper_bridge.libmapper_dev.add_output_signal('dummy', 1, 'i')

    while 1:
        libmapper_bridge.libmapper_dev.poll(50)
        dummy.update(10)
        print("signal updated")
        time.sleep(2)
