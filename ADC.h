/*
 * File:   ADC.h
 * Author: Jordan Ross
 *
 * Created on February 12, 2014, 8:17 AM
 */

 
#define AUDIO_CH 0  //ch 0 = stereo jack input
#define AUDIO_MASK 0xfffe // AN4 as analog input
void InitADC(int channelMask);
int ReadADC(int ch);