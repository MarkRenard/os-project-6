// pcb.h was created by Mark Renard on 4/30/2020.
//
// This file defines the page table entry struct and a process control block 
// specific to assignment 6.

#ifndef PCB_H
#define PCB_H

#include <stdbool.h>
#include <sys/types.h>

#include "clock.h"
#include "constants.h"

// Defines an entry in the page table of each process
typedef struct pageTableEntry{
	char valid;
	char dirty;
	unsigned char frameNumber;
} PageTableEntry;

// Defines types of reference a process can make
typedef enum RefType {READ_REFERENCE, WRITE_REFERENCE} RefType;

// Stores the virtual address, type, and start and times of a reference
typedef struct reference {
	int address;		// The referenced virtual address
	RefType type;		// Whether the reference is read or write

	Clock startTime;	// The time the memory reference started
	Clock endTime;		// The time the reference was comlpeted
	bool endTimeIsSet;	// Whether endTime has been set
} Reference;

struct queue;

// Defines the process control block structure
typedef struct pcb{
	int simPid;			// The simPid of the process
	pid_t realPid;			// The actual pid of the process

	// Page table for the process
	PageTableEntry pageTable[MAX_ALLOC_PAGES];
	int lengthRegister;		// The number of allocated pages

	// The last memory reference the process made
	Reference lastReference;

	// Statistics
	Clock totalAccessTime;		// Total time spent accessing memory
	unsigned int totalReferences;	// Total number of memory references

	// Fields used in Queue for paging I/O
	struct queue * currentQueue;	// Queue the pcb is currently in
	struct pcb * next;		// Next pcb in current queue
	struct pcb * previous;		// Previous pcb in queue

} PCB;

// Function prototypes
void initPcb(PCB *, int simPid);
int getFreePcbIndex(PCB * pcbs);
void initPcbArray(PCB *);
void resetPcb(PCB *);
void setLastReferenceInPcb(PCB *, int address, RefType type, Clock startTime);
void setIoCompletionTimeInPcb(PCB * pcb, Clock endTime);
void completeReferenceInPcb(PCB * pcb, Clock refCompletionTime);
Clock getEatFromPcb(const PCB * pcb);

#include "queue.h"
#endif

