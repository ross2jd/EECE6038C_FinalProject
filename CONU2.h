/* 
 * File:   CONU2.h
 * Author: cfrosty13
 *
 * Created on January 29, 2014, 7:53 PM
 */

#ifndef CONU2_H
#define	CONU2_H

#include "xc.h"

#define CTS     _RF12   // Clear to send, in, handshake
#define RTS     _RF13   // Request to Send, out, handshake
#define TRTS    TRISFbits.TRISF13
#define BACKSPACE 0x8

#define BRATE   34  // 115200 BaudRate (BREGH = 1)
#define U_ENABLE 0x8008 // enable the UART peripheral
#define U_TX    0x0400  // enable transmission

void InitU2(void);
int putU2(int c);
void putsU2(char *s);
char getU2(void);
char *getsnU2(char *s, int len);

#define Clrscr() putsU2("\x1b[2J")
#define Home()  putsU2("\x1b[1;1H")
#define pcr()   putU2('\r'); putU2('\n')

#endif	/* CONU2_H */

