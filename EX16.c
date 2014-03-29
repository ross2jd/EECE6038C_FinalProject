#include "EX16.h"

void InitEX16(void)
{
    // Do nothing right now!
}

void Delayms(unsigned t)
{
    T3CON = 0x8000;
    while(t--)
    {
        TMR3 = 0;
        while(TMR3 < (FCY/1000));
    }
}
