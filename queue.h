// queue.h was created by Mark Renard on 2/5/2020 and modified on 4/29/2020.
// This file defines function prototypes for a queue structure

#ifndef QUEUE_H
#define QUEUE_H

#include "pcb.h"
#include <stdio.h>

typedef struct queue {
	PCB * back;
	PCB * front;
	int count;
} Queue;

void printQueue(FILE *, const Queue *);
void addToFront(Queue * q, PCB * pcb);
void initializeQueue(Queue *);
void enqueue(Queue *, PCB *);
PCB * dequeue(Queue *);
void removeFromCurrentQueue(PCB *);

#endif

