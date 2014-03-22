/*
**
**	Buttons read and debounce
*/

#include "p24fj128ga010.h"
#include "EX16.h"
#include "buttons.h"

int ReadKEY( void)
{   // returns 0.F if key pressed, 0 = none
    int c = 0;

    if ( !_RD6) // leftmost button 
        c |= 8;
    if ( !_RD7)
        c |= 4;
    if ( !_RA7)
        c |= 2;
    if ( !_RD13) // rightmost button
        c |= 1;

    return c;
} // readKEY


int GetKEY( void)
{   // wait for a key pressed and debounce
    int i=0, r=0, j=0;
    int c;

    // wait for a key pressed for at least .1sec
    do{
        Delayms( 10);
        if ( (c = ReadKEY()))
        {
            if ( c>r)       // if more than one button pressed
                r = c;      // take the new code
            i++;    
        }
        else 
            i=0;
    } while ( i<10);

    // wait for key released for at least .1 sec
    i =0;
    do {
        Delayms( 10);   
        if ( (c = ReadKEY()))
        {
            if (c>r)        // if more then one button pressed 
                r = c;      // take the new code
            i=0;            
            j++;            // keep counting to detect long button hold
        }
        else 
            i++;
    } while ( i<10);        
    
    // check if a button was pushed longer than 500ms
    if ( j>80) 
        r+=0x80;

    return r;
} // getKEY
