/* 
 * File:   EX16.h
 * Author: cfrosty13
 *
 * Created on February 12, 2014, 8:02 AM
 */

#ifndef EX16_H
#define	EX16_H

#include <p24FJ128GA010.h>

#define FCY 16000000UL  // instruction clock 16 MHz

void InitEX16(void);
void Delayms(unsigned t);

#endif	/* EX16_H */

