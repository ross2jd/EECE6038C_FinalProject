/* 
 * File:   wav.h
 * Author: cfrosty13
 *
 * Created on April 16, 2014, 8:19 AM
 */

#ifndef WAV_H
#define	WAV_H

typedef struct {
    char riffHeader[4];         // Makes the file as a riff file
    unsigned long fileSize;     // Size of the overall file
    char waveHeader[4];         // File type header
    char fmtHeader[4];          // Format chunk marker - includes trailing null
    unsigned long fmtDataLen;   // Length of format data
    unsigned fmtType;           // Type of format (1 is PCM)
    unsigned numChannels;       // Number of channels
    unsigned long sampleRate;   // 32 byte integer
    unsigned long byteRate;     // (SampleRate*BitsPerSample*Channels)/8
    unsigned blockAlign;        // (NumChannels*BitsPerSample)/8
    unsigned bitsPerSample;     // 8 bits = 8, 16 bits = 16, etc.
    char dataID[4];             // "data" chunk header
    unsigned long dataSize;     // Size of the data section
} WAV;

WAV* initWavStruct(unsigned long dataSize);

#endif	/* WAV_H */

