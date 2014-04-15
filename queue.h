#include<stdio.h>

#define QUEUE_SIZE  1250

void initQueue();
void clearQueue(int choice);
void resetQueue(int choice);
int getQueuePosition(int choice);
int enqueue(int data, int choice);
int* getQueue(int choice);
int queueIsFull(int choice);
