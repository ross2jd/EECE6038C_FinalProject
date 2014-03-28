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

// Declare global variables
#define PERIOD  5
#define BUFFER_SIZE 256

// Timer1 ISR
void _ISR _T1Interrupt(void)
{
    // If the buffer is full stop the timer and print an error message.
    if (isFull())
    {
        _T1IE = 0;
        printf("Error:: Queue is full!");
    }
    else
    {
        // Read the ADC and enqueue it into the buffer.
        Enqueue(ReadADC(TEMP_CH));
    }
    _T1IF = 0; // clear flag
}

int main(void) {
    // Declare local variables
    int keyPressed = 0;
    int writeBuffer[BUFFER_SIZE];
    int curBuffPos = 0;

    // initializations
    InitU2();   // 115,200 baud 8, n ,1
    printf("Initializing system...\n");
    InitADC(TEMPMASK);
    InitQueue();
    // Init the timer
    _T1IP = 4;      // default priority level
    TMR1 = 0;       // clear the value
    PR1 = PERIOD-1; // Set the period register
    T1CON = 0x0020; // disabled, prescaler 1:64, internal clock
    _T1IF = 0;      // clear the flag for the interrupt
    printf("Done initializing\n");

    // Open up file for writing the raw data.

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
        printf("Exiting...");
        return 0;
    }

    // Start the timer
    T1CONbits.TON = 1;
    _T1IE = 0;
    while (1)
    {
        if (!isEmpty())
        {
            // If the queue is not empty
            writeBuffer[curBuffPos] = Dequeue();
            curBuffPos++;
            if (curBuffPos == BUFFER_SIZE)
            {
                // The buffer is full, perform a write and then reset the
                // current buffer position.
                curBuffPos = 0;
            }
        }
        keyPressed = ReadKEY();
        if (keyPressed & 4)
        {
            // The user has signaled the song is complete. Disable the timer
            // and then clear out the queue and perform the final write.
            _T1IE = 0;
            while(!isEmpty())
            {
                writeBuffer[curBuffPos] = Dequeue();
                curBuffPos++;
                if (curBuffPos == BUFFER_SIZE)
                {
                    // The buffer is full, perform a write and then reset the
                    // current buffer position.
                    curBuffPos = 0;
                }
            }
            // perform on final write of curBuffPos size.
            printf("Done!");
            return 0;
        }
    }
    return 0;
}
