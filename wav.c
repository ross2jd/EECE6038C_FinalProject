#include "wav.h"
#include <stdlib.h>     // malloc...

WAV* initWavStruct(unsigned long dataSize)
{
    WAV* wav;

    wav = (WAV *) malloc(sizeof(WAV));
    wav->riffHeader[0] = 'R';
    wav->riffHeader[1] = 'I';
    wav->riffHeader[2] = 'F';
    wav->riffHeader[3] = 'F';
    wav->fileSize = dataSize + 44;
    wav->waveHeader[0] = 'W';
    wav->waveHeader[1] = 'A';
    wav->waveHeader[2] = 'V';
    wav->waveHeader[3] = 'E';
    wav->fmtHeader[0] = 'f';
    wav->fmtHeader[1] = 'm';
    wav->fmtHeader[2] = 't';
    wav->fmtHeader[3] = 0x20;
    wav->fmtDataLen = 16;
    wav->fmtType = 1;
    wav->numChannels = 1;
    wav->sampleRate = 44100;
    wav->byteRate = 88200;
    wav->blockAlign = 2;
    wav->bitsPerSample = 16;
    wav->dataID[0] = 'd';
    wav->dataID[1] = 'a';
    wav->dataID[2] = 't';
    wav->dataID[3] = 'a';

    return wav;
}