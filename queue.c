// queue.c was created by Mark Renard on 2/5/2020 and modified on 4/29/2020.
// This file defines functions that operate on a queue of pcbs.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"
#include "perrorExit.h"

// Sets the values of pointers to null and count to 0
void initializeQueue(Queue * qPtr){
	qPtr->front = NULL;
	qPtr->back = NULL;
	qPtr->count = 0;
}

// Prints the simPid of pcbs in the queue from front to back
void printQueue(FILE * fp, const Queue * q){
	PCB * pcb = q->front;

	while (pcb != NULL){
		fprintf(fp, " %02d", pcb->simPid);
		pcb = pcb->previous;
	}
}

// Adds a pcb to the front of the queue
void addToFront(Queue * q, PCB * pcb){
	if (pcb->currentQueue != NULL)
		perrorExit("addToFront on pcb with non-null currentQueue");

	pcb->next = NULL;
	pcb->previous = q->front;

	if (pcb->previous != NULL)
		pcb->previous->next = pcb;

	q->front = pcb;

	// Adds to back as well if previously empty
	if (q->back == NULL) q->back = pcb;
	q->count++;
	
}

// Adds a pcb to the back of the queue
void enqueue(Queue * q, PCB * pcb){

	if (pcb->currentQueue != NULL)
		perrorExit("Tried to enqueue pcb with non-null currentQueue");

	pcb->currentQueue = q;

	// Adds pcb to queue	
	if (q->back != NULL){

		// Connects to back of queue if queue is not empty
		q->back->previous = pcb;
		pcb->next = q->back;
	} else {
		// Adds to front if queue is empty
		q->front = pcb;
		pcb->next = NULL;
	}
	q->back = pcb;

	// Back of queue shouldn't have a previous element
	q->back->previous = NULL;

	// Increments node count in queue
	q->count++;

	printQueue(stderr, q);
	fprintf(stderr, "\n");	

}

// Removes and returns PCB reference from the front of the queue
PCB * dequeue(Queue * q){

	// Exits with an error pcb if queue is empty
	if (q->count <= 0 || q->front == NULL || q->back == NULL)
		perrorExit("Called dequeue on empty queue");

	// Assigns current front of queue to returnVal
	PCB * returnVal = q->front;  	
	
	// Removes the front node from the queue
	q->front = q->front->previous; // Assigns new front of queue
	if (q->front != NULL)
		q->front->next = NULL;	// Front of queue shouldn't have a next
	else
		q->back = NULL;	// Back is null if queue is empty

	// Removes links from dequeued element
	returnVal->previous = NULL; 
	returnVal->next = NULL;
	returnVal->currentQueue = NULL;

	q->count--;
	return returnVal;

}

// Removes a particular element from any place in its queue
void removeFromCurrentQueue(PCB * pcb){
	Queue * q = pcb->currentQueue;

	if (q == NULL) 
		perrorExit("Tried to remove pcb when not in queue");

	if (q->front == NULL || q->back == NULL || q->count < 1)
		perrorExit("Tried to remove pcb from empty queue");

	// Finds new front if pcb is the front
	if (q->front == pcb)
		q->front = pcb->previous;

	// Finds new back if pcb is the back
	if (q->back == pcb)
		q->back = pcb->next;

	// Connects previous node to next node
	if (pcb->next != NULL)
		pcb->next->previous = pcb->previous;

	// Connects next node to previous node
	if (pcb->previous != NULL)
		pcb->previous->next = pcb->next;

	// Sets pcb queue pointers to null
	pcb->next = NULL;
	pcb->previous = NULL;
	pcb->currentQueue = NULL;
	
	q->count--;
}
