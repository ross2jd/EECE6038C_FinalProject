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
#define PERIOD  15
#define BUFFER_SIZE 1024
int keyPressed = 0;

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
        Enqueue(0);//ReadADC(AUDIO_CH));
    }
    _T1IF = 0; // clear flag
}

void _ISR _ADC1Interrupt(void)
{
    //printf(".");
    keyPressed = ReadKEY();
    if (keyPressed & 8){
        printf("done\n");
        IEC0bits.AD1IE = 0;
    }
    else{
        printf("%i\n", ADC1BUF0);
        IFS0bits.AD1IF = 0;
    }
}

int main(void) {
    // Declare local variables
    int writeBuffer[BUFFER_SIZE];
    int curBuffPos = 0;
    int numWritesCounter = 0;

    // initializations
    InitU2();   // 115,200 baud 8, n ,1
    Clrscr();
    Home();
    printf("Initializing system...\n");
    InitADC(AUDIO_MASK);
    InitQueue();
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
    keyPressed = 0;
    // Enable the interrupts for the ADC
    IFS0bits.AD1IF = 0;
    IEC0bits.AD1IE = 1;
    // Start the automatic sample & conversion process
    AD1CON1bits.ASAM = 1;
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

                // Every 150 reads lets show that the operation is in progress.
                // This is roughly about 1 second if the interrupts are
                // correct.
                numWritesCounter++;
                if (numWritesCounter >= 2345)
                {
                    numWritesCounter = 0;
                    printf(".");
                }

            }
        }
        /*keyPressed = ReadKEY();
        if (!(keyPressed & 8))
        {
            // Close up the file.
            keyPressed = 0;
            printf("Exiting...");
            return 0;
        }
        keyPressed = 0;*/
        
        if (IEC0bits.AD1IE == 0){
            while(1);
                //printf("done");
        }
        /*
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
            printf("\nDone!");
            while(1);
        }*/
    }
    //while(1);
    return 0;
}
