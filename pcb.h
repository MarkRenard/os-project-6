// pcb.h was created by Mark Renard on 4/30/2020.
//
// This file defines the page table entry struct and a process control block 
// specific to assignment 6.

#ifndef PCB_H
#define PCB_H

#include <stdbool.h>
#include <sys/types.h>
#include "constants.h"

// Defines an entry in the page table of each process
typedef struct pageTableEntry{
	char valid;
	char dirty;
	unsigned char frameNumber;
} PageTableEntry;

struct queue;

// Defines the process control block structure
typedef struct pcb{
	int simPid;			// The simPid of the process
	pid_t realPid;			// The actual pid of the process

	// Page table for the process
	PageTableEntry pageTable[MAX_ALLOC_PAGES];
	int lengthRegister;		// The number of allocated pages

	// Fields used in Queue for paging I/O
	struct queue * currentQueue;	// Queue the pcb is currently in
	struct pcb * next;		// Next pcb in current queue
	struct pcb * previous;		// Previous pcb in queue

} PCB;

void initPcb(PCB *, int simPid);
int assignFreePcb(PCB *, pid_t);
void initPcbArray(PCB *);
void resetPcb(PCB *);

#include "queue.h"
#endif

