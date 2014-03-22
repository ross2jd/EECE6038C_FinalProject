/*
 * File:   ADC.h
 * Author: Jordan Ross
 *
 * Created on February 12, 2014, 8:17 AM
 */
 
 
#define TEMP_CH 4   //ch 4 = TC1047 temperature sensor
#define TEMPMASK 0xffef // AN4 as analog input
void InitADC();
int ReadADC(int ch);