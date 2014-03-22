/* 
 * File:   LCD.c
 * Author: Jordan Ross
 *
 * Created on February 12, 2014, 4:40 PM
 */

#include "LCD.h"

void InitLCD(void)
{
    // PMP Initialization
    PMCON = 0x83BF;     // Enable the PMP, long waits
    PMMODE = 0x3FF;     // Master mode 1
    PMAEN = 0x0001;     // PMA0 enabled

    // Init TMR1
    T1CON = 0x8030;     // Fosc/2, 1:256, 16us/tick
    // Wait for > 30 ms
    TMR1 = 0;
    while(TMR1 < 2000); // 2000 x 16 us/tick = 32 ms

    PMADDR = LCDCMD;    // command register
    PMDATA = 0b00111000;   // 8-bit, 2 lines, 5 x 7
    TMR1 = 0;
    while(TMR1 < 3); // 3 x 16 us/tick = 48 us

    PMDATA = 0b00001100;    // ON, cursor off, blink off
    TMR1 = 0;
    while(TMR1 < 3); // 3 x 16 us/tick = 48 us

    PMDATA = 0b00000001;    // clear display
    TMR1 = 0;
    while(TMR1 < 100); // 100 x 16 us/tick = 1.6 ms

    PMDATA = 0b00000110;    // increment cursor
    TMR1 = 0;
    while(TMR1 < 100); // 100 x 16 us/tick = 1.6 ms
}

char ReadLCD(int addr)
{
    int dummy;
    while(PMMODEbits.BUSY);     // wait for PMP to be available
    PMADDR = addr;              // select the command address
    dummy = PMDATA;             // initiate a read cycle, dummy
    while(PMMODEbits.BUSY);     // wiat for PMP to be available
    return(PMDATA);             // read the status register
}

void WriteLCD(int addr, char c)
{
    while(BusyLCD());
    while(PMMODEbits.BUSY); // wait for PMP to be available
    PMADDR = addr;
    PMDATA = c;
}

void putsLCD(char *s)
{
    while(*s)
        putLCD(*s++);
}
