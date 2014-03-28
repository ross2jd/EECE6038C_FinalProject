/*
 ** write.c
 ** replaces stdio lib write function with UART2
 */

#include <xc.h>
#include <stdio.h>
#include "CONU2.h"

int write(int handle, void *buffer, unsigned int len)
{
    int i;

    switch (handle)
    {
        case 0:
        case 1:
        case 2:
            i = len;
            while(i--)
                putU2(*(char*)buffer++);
            break;
        default:
            break;
    }
    return (len);
}