#include <string.h>
#include "queue.h"

int queue[QUEUE_SIZE];
int queue2[QUEUE_SIZE];

int queuePos;
int queuePos2;

void initQueue()
{
    resetQueue(0);
    resetQueue(1);
    clearQueue(0);
    clearQueue(1);
}

void resetQueue(int choice)
{
    if (choice == 0)
    {
        queuePos = 0;
    }
    else
    {
        queuePos2 = 0;
    }
}

void clearQueue(int choice)
{
    if (choice == 0)
    {
         memset(queue,0,QUEUE_SIZE*sizeof(queue[0]));
    }
    else
    {
         memset(queue2,0,QUEUE_SIZE*sizeof(queue2[0]));
    }
}

int getQueuePosition(int choice)
{
    if (choice == 0)
    {
        return queuePos;
    }
    else
    {
        return queuePos2;
    }
}

int enqueue(int data, int choice)
{
    if (choice == 0)
    {
        if (queuePos >= (QUEUE_SIZE))
            return 0;
        queue[queuePos] = data;
        queuePos++;
        return 1;
    }
    else
    {
        if (queuePos2 >= (QUEUE_SIZE))
            return 0;
        queue2[queuePos2] = data;
        queuePos2++;
        return 1;
    }
}

int* getQueue(int choice)
{
    if (choice == 0)
        return queue;
    else
        return queue2;
}

int queueIsFull(int choice)
{
    if (choice == 0)
    {
        if (queuePos == QUEUE_SIZE)
            return 1;
        else
            return 0;
    }
    else
    {
        if (queuePos2 == QUEUE_SIZE)
            return 1;
        else
            return 0;
    }

}
