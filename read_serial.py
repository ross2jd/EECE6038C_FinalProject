__author__ = 'bbutsch'

# import csv
import serial
import os
import sys
import time


PICport = 'COM4'

# define the name of the connection, but don't set it up just yet - for exception handling
pic = serial.Serial()

# Open up the serial connection for the DMM.
try:
    pic = serial.Serial(port=PICport, baudrate="115200", bytesize=serial.EIGHTBITS,
                        parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE)
except Exception as e:  # catch *all* exceptions
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
    if pic.inWaiting():
        #print pic.readline()
        #if (len(buffer) == 2):
        #    start = time.time()
        #    print "Recording..."
        
        val = pic.readline().rstrip()
        #print val
        buffer.append(val)
        if val == "done":
            isdone = 0
        #print val
        
        # pic.flushInput()


#print "Elapsed time of recording...", (start-end)
        
print("Done getting data from serial\nSaving data to file...")
fh = open("data.txt", "w")
for data in buffer:
    fh.write(data + '\n')
    #if data[1].isalnum():
    #    fh.write(data[0] + "," + data[1] + '\n')


fh.close()
print("Done!")