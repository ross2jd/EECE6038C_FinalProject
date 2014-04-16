/*
 * File:   main.c
 * Author: Jordan Ross
 *
 * Created on March 24, 2014, 5:12 PM
 */


#include "config.h"
#include "EX16.h"
#include "ADC.h"
#include "SDMMC.h"
#include "CONU2.h"
#include "fileio.h"
#include "queue.h"
#include "buttons.h"
#include "wav.h"
#include <stdio.h>

// Declare global variables
#define PERIOD  15
int keyPressed = 0;
int curQueue;
int recording;

extern int queue[QUEUE_SIZE];
extern int queue2[QUEUE_SIZE];

// Timer3 ISR
void _ISRFAST _T3Interrupt(void)
{
    if (queueIsFull(curQueue))
    {
        // Switch out the queues
        curQueue = (curQueue+1)%2;
        if (queueIsFull(curQueue))
        {
            // Switch out the queues
            curQueue = (curQueue+1)%2;
            // Error out
            //printf("Error:: Both queue is full");
            //while(1);
        }
    }
    // Get the value from the ADC and store it in the correct queue
    enqueue((ReadADC(AUDIO_CH)-518)*250, curQueue);

    _T3IF = 0; // clear flag
}

// Button interrupt
void _ISRFAST _CNInterrupt(void)
{
    keyPressed = GetKEY();
    if ((keyPressed & 8))
    {
        // Close up the file.
        keyPressed = 0;
        recording = 0;
        printf("Done...\n");
    }
    _CNIF = 0;
}

int main(void) {
    // Declare local variables
    MFILE *fd, *fs;
    unsigned i, r;
    unsigned queueZeroFill = 0;
    unsigned queueOneFill = 0;
    unsigned file_num;
    char file_name[11];
    char wavFileName[12];
    WAV* wavHandle;

    // initializations
    InitU2();   // 115,200 baud 8, n ,1
    Clrscr();
    Home();
    printf("Initializing system...\n");
    InitADC(AUDIO_MASK);

    // Set the first queue to be filled as the queue 0
    curQueue = 0;
    // Init the queue
    initQueue();

    // Initialize Timer 3
    _T3IP = 4;
    TMR3 = 0;
    // We want to interrupt every 1/44100Hz ~ 22.6757 uS.
    // We have the constraint that the conversion process takes 1.5 uS
    // Thus we want our timer to interrupt ever 22.6757uS - 1.5uS = 21.1757uS
    // Then using the equation:
    // Desired_Time = PERIOD*PRESCALE*1cycle/16Mhz
    // We solve for PERIOD*PRESCALE and plug in 21.1757uS for Desired_Time we get,
    // PERIOD*PRESCALE = 338.81
    // Thus we choose PRESCALE = 64 & PERIOD = 5
    T3CON = 0x0020; // disabled, prescaler 1:64, internal clock
    PR3 = 5 - 1;    // interrupt every 20 uS
    _T3IF = 0;

    // Code for setting up the SD Card
    while(!DetectSD()); // assumes SDCD pin is by default an input
    Delayms(100);       // wait for card to power up
    printf("Media detected\n");
    if (!mount())
    {
        printf("Mount failed...\n");
        while(1);
    }
    printf("Media mounted\n");

    // Code for opening the file
    file_num = -1;
    fd = NULL;
    while (fd == NULL)
    {
        file_num++;
        if (file_num >= 99)
        {
            // Error out
            unmount();
            printf("Please clear out the song files on the SD card\n");
            while(1);
        }
        sprintf(file_name, "song%d.txt", file_num);
        fd = fopenM(file_name, "w");
    }
    printf("File will be stored in: %s", file_name);
    printf("File opened\n");

    printf("Done initializing\n");

    // Print the instructional message
    printf("Please hook up your music player to the stereo jack. When\n");
    printf("you are ready you should press play and the RD6 pushbutton\n");
    printf("close to the same time.\n");
    // Wait for the user to press the button RD6
    keyPressed = GetKEY();
    // If the user pressed any button BUT RD6 exit the system.
    if (!(keyPressed & 8))
    {
        // Close up the file.
        keyPressed = 0;
        printf("Exiting...\n");
        return 0;
    }
    
    printf("Beginning recording...\n");

    keyPressed = 0;

    // Set recording to on
    recording = 1;

    // Enable the button interrupts
    _CNIE = 1;
    _CN15IE = 1;

    // Enable the timers
    _T3IE = 1;
    T3CONbits.TON = 1;
    while(recording)
    {
        if (queueIsFull(0))
        {
            // We should write the contents of queue one
            //writeQueue = getQueue(0);
            // We have a problem here where we are writing characters and not
            // characters... Timing works though!
            queueZeroFill += 1;
            r = fwriteM(getQueue(0), QUEUE_SIZE*2, fd);
            resetQueue(0);
            // write the data for write queue
        }
        else if (queueIsFull(1))
        {
            // We should write the contents of queue two.
            //writeQueue = getQueue(1);
            queueOneFill += 1;
            r = fwriteM(getQueue(1), QUEUE_SIZE*2, fd);
            resetQueue(1);
        }
    }
    // Disable timer3 and interrupt
    _T3IE = 0;
    T3CONbits.TON = 0;
    
    // Now need to write the rest of the data to the file
    r = fwriteM(getQueue(curQueue), getQueuePosition(curQueue)*2, fd);
    
    printf("Done recording the audio\n");
    // Close up the file.
    fcloseM(fd);
    
    // Now we are going to add in the WAV header
    printf("Creating the WAV file...\n");

     // init the queues so we can use as a buffer
    initQueue();

    fs = fopenM(file_name, "r");
    wavHandle = initWavStruct(fs->size);

    sprintf(wavFileName, "song%d.wav", file_num);
    fd = fopenM(wavFileName, "w");
    if (fd == NULL)
    {
        printf("Failed to open file!\n");
        fcloseM(fs);
        unmount();
        while(1);
    }

    r = fwriteM(wavHandle, sizeof(WAV), fd);
    // Copy the raw data to the WAV file
    do {
        r = freadM(queue, QUEUE_SIZE, fs);
        r = fwriteM(queue, r, fd);
    }while(r == QUEUE_SIZE);

    fcloseM(fd);
    fcloseM(fs);
    printf("Done creating the WAV file!\n");
    printf("The WAV file is stored in %s\n", wavFileName);

    unmount();
    printf("File is closed, SD Card unmounted\n");

    printf("We wrote to queue_0 %d times and queue_1 %d times\n", queueZeroFill, queueOneFill);

    while(1);
    return 0;
}
