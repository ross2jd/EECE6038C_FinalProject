#include "EX16.h"

void InitEX16(void)
{
    // Do nothing right now!
}

void Delayms(unsigned t)
{
    T2CON = 0x8000;
    while(t--)
    {
        TMR2 = 0;
        while(TMR2 < (FCY/1000));
    }
}
