/* 
 * File:   SDMMC.h
 * Author: Jordan Ross
 *
 * Created on March 5, 2014, 5:49 PM
 */

#ifndef SDMMC_H
#define	SDMMC_H

#define FALSE   0
#define TRUE    !FALSE
#define FAIL    0

// I/O definitions
#define SD_LED      _RA0
#define READ_LED    _RA1
#define WRITE_LED   _RA2

typedef unsigned long LBA;  // logic block address, 32 bit wide

void InitSD(void);      // initailizes the I/O pins
int InitMedia(void);    // intializes the SD/MMC device

int DetectSD(void);     // detects the card precense
int DetectWP(void);     // detects the write protect switch

int ReadSECTOR(LBA, unsigned char *);   // reads a block of data
int WriteSECTOR(LBA, unsigned char *);  // writes a block of data


#endif	/* SDMMC_H */

