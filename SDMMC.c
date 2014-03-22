/*
 * File:   SDMMC.c
 * Author: Jordan Ross
 *
 * Created on March 5, 2014, 5:00 PM
 */


#include "xc.h"
#include "SDMMC.h"

// I/O definitions for AV16/32
#define SDWP    _RG1    // Write Protect input
#define SDCD    _RF1    // Card detect input
#define SDCS    _RF0    // Card select output

// Card select TRIS control
#define TRISCS()    _TRISF0=0

// SPI port selection
#define SPIBUF  SPI2BUF
#define SPICON  SPI2CON1
#define SPISTAT SPI2STAT
#define SPIRFUL SPI2STATbits.SPIRBF

void InitSD(void)
{
    SDCS = 1;
    TRISCS();   // make Card select an output pin

    // init the spi module for a slow (safe) cllock speedfirst
    SPICON = 0x013c;    // CKE = 1; CKP = 0, sample middle, 1:64
    SPISTAT = 0x8000;   // enable the peripheral
}

// send one byte of data and receive one back at the same time
unsigned char WriteSPI(unsigned char b)
{
    SPIBUF = b; // write to buffer for TX
    while(!SPIRFUL);    // wait until transfer complete
    return SPIBUF;  // read the recieved value;
}

#define ReadSPI()   WriteSPI(0xFF)
#define ClockSPI()  WriteSPI(0xFF)

// SD card commands
#define RESET           0   // a.k.a. GO_IDLE(CMD0)
#define INIT            1   // a.k.a. SEND_OP_COND(CMD1)
#define READ_SINGLE     17  // read a block of data
#define WRITE_SINGLE    24  // write a block of data

#define DisableSD() SDCS = 1; ClockSPI()
#define EnableSD()  SDCS = 0

// Sends a 6 byte command block to the card and leaves SDCS active
int SendSDCmd(unsigned char c, LBA a)
{
    int i, r;

    // enable SD card
    EnableSD();

    // send a command packet (6 bytes)
    WriteSPI(c|0x40);   // send command + frame bit
    WriteSPI(a>>24);    // MSB of the address
    WriteSPI(a>>16);
    WriteSPI(a>>8);
    WriteSPI(a);        // LSB
    WriteSPI(0x95);     // send CMD0 (RESET) CRC

    // now wait for a response (allow up to 8 bytes delay)
    i = 9;
    do {
        r = ReadSPI();  // check if ready
        if (r != 0xFF) break;
    } while(--i > 0);

    return(r);

    /* return response
     FF - timeout, no answer
     00 - command accepted
     01 - command received, card in idle state (after RESET)
     */
}

int InitMedia(void)
{
    int i, r;

    // while the card is not selected
    DisableSD();

    // send 80 clock cycles to start up
    for (i=0; i<10; i++)
        ClockSPI();

    // then select the card
    EnableSD();

    // send a reset command to enter SPI mode
    r = SendSDCmd(RESET, 0); DisableSD();
    if (r != 1)
    {
        return 0x84;
    }

    // send repeatedly INIT
    i = 10000;  // allow for up to 0.3s before timeout
    do {
        r = SendSDCmd(INIT, 0); DisableSD();
        if (!r) break;
    } while(--i > 0);
    if (i == 0)
        return 0x85;    // timed out

    // increase speed
    SPISTAT = 0;        // diable momentarily the SPI module
    SPICON = 0x013b;    // change prescaler to 1:2
    SPISTAT = 0x8000;   // re-enable the SPI module

    return 0;
} // init media

// SD card responses
#define DATA_START  0xFE

int ReadSECTOR(LBA a, unsigned char *p)
{
    // a        LBA requested
    // p        pointer to data buffer
    // returns  TRUE if successful

    int r, i;

    READ_LED = 1;

    r = SendSDCmd(READ_SINGLE, (a << 9));
    if (r == 0) // check if the command was accepted
    {
        // wait for a response
        i = 25000;
        do {
            r = ReadSPI();
            if (r == DATA_START) break;
        } while(--i>0);

        // if it did not timeout, read a 512 byte sector of data
        if(i)
        {
            for(i=0; i<512; i++)
                *p++ = ReadSPI();

            // ignore CRC
            ReadSPI();
            ReadSPI();
        } // data arrived
    }  // command accepted

    // remember to diable the card
    DisableSD();
    READ_LED = 0;

    return (r == DATA_START);   // return TRUE if successful
} // ReadSECTOR

#define DATA_ACCEPT 0x05

int WriteSECTOR(LBA a, unsigned char *p)
{
    // a        LBA of sector requested
    // p        pointer to sector buffer
    // returns  TRUE if successful
    unsigned r, i;

    WRITE_LED = 1;

    r = SendSDCmd(WRITE_SINGLE, (a<<9));
    if (r == 0) // check if command was accepted
    {
        WriteSPI(DATA_START);

        for (i=0; i<512; i++)
            WriteSPI(*p++);

        // send dummy CRC
        ClockSPI();
        ClockSPI();

        // check if data accepted
        if( (r = ReadSPI() & 0xf) == DATA_ACCEPT)
        {
            for(i=65000; i>0; i--)
            {
                 // wait for end of write operation
                if ((r = ReadSPI()))
                    break;
            }
        } // accepted
        else
            r = FAIL;
    } // command accepted

    // to diable the card and return
    DisableSD();
    WRITE_LED = 0;

    return (r);     // return TRUE if successful
}

int DetectSD(void)
{
    return(!SDCD);
}

int DetectWP(void)
{
    return(!SDWP);
}
