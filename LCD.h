/* 
 * File:   LCD.h
 * Author: Jordan Ross
 *
 * Created on February 12, 2014, 4:41 PM
 */

#ifndef LCD_H
#define	LCD_H

#include <p24FJ128GA010.h>

#define LCDDATA 1
#define LCDCMD  0
#define PMDATA  PMDIN1

// Prototypes
void InitLCD(void);
char ReadLCD(int addr);
void WriteLCD(int addr, char c);
void putsLCD(char *s);

// Useful macros
#define BusyLCD()   ReadLCD(LCDCMD) & 0x80
#define AddrLCD()   ReadLCD(LCDCMD) & 0x7F
#define getLCD()    ReadLCD(LCDDATA)
#define putLCD(d)   WriteLCD(LCDDATA, (d))
#define CmdLCD(c)   WriteLCD(LCDCMD, (c))
#define HomeLCD()   WriteLCD(LCDCMD, 2)
#define ClrLCD()    WriteLCD(LCDCMD, 1)
#define SetLCDG(a)  WriteLCD(LCDCMD, ((a) & 0x3F) | 0x40)
#define SetLCDC(a)  WriteLCD(LCDCMD, ((a) & 0x7F) | 0x80)

#endif	/* LCD_H */

