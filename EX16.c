#include "EX16.h"

void InitEX16(void)
{
    // Do nothing right now!
}

void Delayms(unsigned t)
{
    T1CON = 0x8000;
    while(t--)
    {
        TMR1 = 0;
        while(TMR1 < (FCY/1000));
    }
}
