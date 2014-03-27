//---------------------------------------------------------------
// File: Queue.h
//
// Header file for the implementation for a queue.
//---------------------------------------------------------------
#ifndef QUEUE_H
#define QUEUE_H

#include <stdio.h>

#define MAX_SIZE    25        // Define maximum length of the queue

// List Function Prototypes
void InitQueue();             // Initialize the queue
void ClearQueue();            // Remove all items from the queue
int Enqueue(int ch);         // Enter an item in the queue
int Dequeue();               // Remove an item from the queue
int isEmpty();                // Return true if queue is empty
int isFull();                 // Return true if queue is full

// Define TRUE and FALSE if they have not already been defined
#ifndef FALSE
#define FALSE (0)
#endif
#ifndef TRUE
#define TRUE (!FALSE)
#endif

#endif
