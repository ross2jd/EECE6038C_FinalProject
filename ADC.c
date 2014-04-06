/*
 * File:   ADC.c
 * Author: Jordan Ross
 *
 * Created on February 12, 2014, 8:17 AM
 */

#include "xc.h"

void InitADC(int channelMask)
{
    /*
    AD1PCFG = channelMask;  // Select the desired chanel as analog
    AD1CON1 = 0x00E0;
    AD1CSSL = 0;            // No scanning.
    AD1CON3 = 0x1F01;//3F;       // Sample time = 31Tad, Tad = 64*Tcy
    AD1CON2 = 0x0000;       // Set AD1IF every sample.
    AD1CON1bits.ADON = 1;   // start the ADC

    // Explorer 16 Development Board Errate (work around 2)
    // RB15 should always be a digital output.
    _LATB15 = 0;
    _TRISB15 = 0;
     */
    AD1PCFG = 0xFFFE; // AN2 as analog, all other pins are digital
    AD1CON1 = 0x00E0; // SSRC bit = 111 implies internal counter
    // ends sampling and starts converting.
    AD1CHS = 0x0000; // Connect RB2/AN2 as CH0 input..
    // in this example RB2/AN2 is the input
    AD1CSSL = 0;
    AD1CON3 = 0x0100; // 0x1000; // Sample time = 15Tad, Tad = Tcy
    AD1CON2 = 0x0000; // Set AD1IF after every each samples
    AD1CON1bits.ADON = 1; // turn ADC ON

}

int ReadADC(int ch)
{
    AD1CHS = ch;        // select analog input channel
    // start sampling, automatic conversion will follow
    AD1CON1bits.SAMP = 1;
    while(!AD1CON1bits.DONE);   // wait to complete conversion
    AD1CON1bits.DONE = 0;
    return ADC1BUF0;            // read the conversion result
}