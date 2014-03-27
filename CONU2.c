/* 
 * File:   CONU2.c
 * Author: Jordan Ross
 *
 * Created on January 29, 2014, 7:53 PM
 */

#include "CONU2.h"

void InitU2(void)
{
    U2BRG = BRATE;
    U2MODE = U_ENABLE;
    U2STA = U_TX;
    RTS = 1;        // set RTS default status
    TRTS = 0;       // make RTS output
}

int putU2(int c)
{
    while(CTS);             //  wait for !CTS, clear to send
    while(U2STAbits.UTXBF);  // wait while Tx buffer is full
    U2TXREG = c;
    return c;
}

void putsU2(char *s)
{
#warning "Fix this!! Commented code out"
    //while(*s)
    //    putU2(*s++);
    //putU2('\r');
    //putU2('\n');
}

char getU2(void)
{
    RTS = 0;
    while(!U2STAbits.URXDA);
    RTS = 1;
    return U2RXREG;
}

char *getsnU2(char *s, int len)
{
    char *p = s;
    do {
        *s = getU2();
        putU2(*s);
        if ((*s==BACKSPACE)&&(s>p))
        {
            putU2(' ');
            putU2(BACKSPACE);
            len++;
            s--;
            continue;
        }
        if (*s == '\n')
            continue;
        if (*s == '\r')
            break;
        s++;
        len--;
    } while(len>1);
    *s = '\0';
    return p;
}

