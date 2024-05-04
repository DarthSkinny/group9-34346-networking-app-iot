#!/usr/bin/env python3

# reading esp32 over usb
# this code reads the water level and temperature sensor reading from the receiving esp32 over USB
import serial
import time

# declare connection parameters
# esp32 is connected to ttyUSB0 which can be detected by running the command ~ls /dev/ttyUSB*
read = serial.Serial(port="/dev/ttyUSB0", baudrate=57600)


# function to create and write text files
def write_to_file(filename, data):
    with open(filename, "w") as file:  # Open file in append mode
        file.write(data)  # Write data with a newline


# while loop keeps the communication open continuously
# if else loop is used to differentiate between the 2 sensor values based on length of received bytes
# the 2 sensor values are stored in 2 separate text files for easy access
while True:
    line = read.readline()
    if 15 <= len(line) <= 17 and line != b'radio_rx  10\r\r\n':
        print("got sensor")
        if len(line) == 15:
            t = line.decode('utf-8')
            print("Storing temperature")
            write_to_file("temperature.txt", t[10:12])
        elif len(line) == 17:
            wl = line.decode('utf-8')
            print("Storing water level")
            write_to_file("water_level.txt", wl[10:14])
    time.sleep(0.1)
