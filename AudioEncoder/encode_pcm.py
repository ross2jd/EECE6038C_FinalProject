__author__ = 'cfrosty13'

import wave
import struct

rawfile = open("data.txt", 'r')
pcmfile = open("tmpPcmFile.pcm", "w")

dcBias = 518;

count = 0
# Convert the data to binary
for line in rawfile:
    line = line.strip()
    if line == "done" or line == "":
        break
    pcmfile.write(str(int(line)-dcBias) + "\n")

rawfile.close()
pcmfile.close()

with open("tmpPcmFile.pcm", 'r') as pcmfile:
    pcmdata = pcmfile.read()

wavefile = wave.open("tempPcmFile.wav", 'w')
wavefile.setparams((1, 2, 50000, 0, 'NONE', 'not compressed'))

values = []
pcmfile = open("tmpPcmFile.pcm", "r")
for line in pcmfile:
    value = int(line.strip())
    packed_value = struct.pack('h', value)
    values.append(packed_value)
    values.append(packed_value)

value_str = ''.join(values)
wavefile.writeframes(value_str)
wavefile.close()