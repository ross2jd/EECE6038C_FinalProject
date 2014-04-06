__author__ = 'bbutsch'

# import csv
import serial
import os
import sys
import time


PICport = 'COM23'

# define the name of the connection, but don't set it up just yet - for exception handling
pic = serial.Serial()

# Open up the serial connection for the DMM.
try:
    pic = serial.Serial(port=PICport, baudrate="115200", bytesize=serial.EIGHTBITS,
                        parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE, timeout=5)
except Exception as e:  # catch *all* exceptions
    # e = sys.exc_info()[0]
    print e
    print ("Serial connection error!")
    sys.exit(-1)

# check if the connection is open
if pic.isOpen():
    print("connected to PIC at " + pic.portstr)
    pic_connected = 1

buffer = []
isdone = 1
while isdone:
    if 1: # pic.inWaiting():
        val = pic.readline().rstrip()
        buffer.append(val) # stores all data received into a list
        # print val
        if val == "done":
            isdone = 0
        # pic.flushInput()

print("Done getting data from serial\nSaving data to file...")
fh = open("mynewfile.txt", "w")
for data in buffer:
    if data.isalnum():
        fh.write(data + '\n')


fh.close()
print("Done!")