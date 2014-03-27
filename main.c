/*
 * File:   main.c
 * Author: Jordan Ross
 *
 * Created on March 24, 2014, 5:12 PM
 */


#include "config.h"
#include "EX16.h"
#include "LCD.h"
#include "SDMMC.h"
#include "fileio.h"

#define B_SIZE  1024
char data[B_SIZE];

int main(void) {
    MFILE *fs, *fd;
    unsigned r;
    unsigned code = 0;

    // initializations
    //InitU2();   // 115,200 baud 8, n ,1
    //putsU2("init");
    InitLCD();
    ClrLCD();
    HomeLCD();
    SetLCDC(0x00);    // cursor to next position
    putsLCD("Init");
    while (!DetectSD());    // assumes SDCD pin is default an input
    Delayms(100);           // wasit for card to power up

    if (mount())
    {
        ClrLCD();
        HomeLCD();
        SetLCDC(0x00);    // cursor to next position
        putsLCD("Mount");
        Delayms(2000);
        code++;
        if ((fs = fopenM("source.txt", "r")))
        {
            ClrLCD();
            HomeLCD();
            SetLCDC(0x00);    // cursor to next position
            putsLCD("source open");
            Delayms(2000);
            if ((fd = fopenM("dest.txt", "w")))
            {
                code++;
                ClrLCD();
                HomeLCD();
                SetLCDC(0x00);    // cursor to next position
                putsLCD("dest open");
                Delayms(2000);
                do {
                    r = freadM(data, B_SIZE, fs);
                    r = fwriteM(data, r, fd);
                } while (r == B_SIZE);
                fcloseM(fd);
                ClrLCD();
                HomeLCD();
                SetLCDC(0x00);    // cursor to next position
                putsLCD("dest open");
                putsLCD("dest closed");
                Delayms(2000);
            }
            else
            {
                ClrLCD();
                HomeLCD();
                SetLCDC(0x00);    // cursor to next position
                putsLCD("dest fail");
                Delayms(2000);
            }

            fcloseM(fs);
            ClrLCD();
            HomeLCD();
            SetLCDC(0x00);    // cursor to next position
            putsLCD("source closed");
            Delayms(2000);
            code++;
        }
        else
        {
            ClrLCD();
            HomeLCD();
            SetLCDC(0x00);    // cursor to next position
            putsLCD("source fail");
            Delayms(2000);
        }
    }
    else
    {
        ClrLCD();
        HomeLCD();
        SetLCDC(0x00);    // cursor to next position
        putsLCD("mount fail");
        Delayms(2000);
    }

    while (1);
    return 0;
}
