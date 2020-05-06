// pcb.c was created by Mark Renard on 4/11/2020 as message.c and modified on
// 4/29/2020.
//
// This file contains functions for initializing and resetting process control
// block structures that contain page tables.

#include "constants.h"
#include "pcb.h"
#include "queue.h"
#include "randomGen.h"

#include <stdbool.h>
#include <stdio.h>

static const Clock MEM_ACCESS_TIME = {MEM_ACCESS_SEC, MEM_ACCESS_NS};

// Sets non-queue values to defaults
static void setDefaults(PCB * pcb){
	pcb->realPid = EMPTY;

	// Unsets valid and dirty indicators in page table
	int i = 0;
	for( ; i < MAX_ALLOC_PAGES; i++){
		pcb->pageTable[i].valid = 0;
		pcb->pageTable[i].dirty = 0;
	}

	// Reference endTime is not set
	pcb->lastReference.completionTimeIsSet = false;

	// Assigns random length
	pcb->lengthRegister = randInt(MIN_ALLOC_PAGES, MAX_ALLOC_PAGES);

	// Initializes statistics
	pcb->totalAccessTime = zeroClock();
	pcb->totalReferences = 0;

}

// Initializes a single pcb to default values
void initPcb(PCB * pcb, int simPid){

	// Sets simPid
	pcb->simPid = simPid;

	// Sets non-queue values to defaults
	setDefaults(pcb);

	// Initializes queue values to null
	pcb->currentQueue = NULL;
	pcb->next = NULL;
	pcb->previous = NULL;
}

// Initializes a pcb not assigned to a running process and returns its simPid
int getFreePcbIndex(PCB * pcbs){
	int simPid;
	for (simPid = 0; simPid < MAX_RUNNING; simPid++){
		if (pcbs[simPid].realPid == EMPTY){
			return simPid;
		}
	}

	// Returns -1 if no pcb exists in the array without an assigned realPid
	return -1;
}

// Initializes the shared array of pcbs to default values
void initPcbArray(PCB * pcbArr){
	int i;
	for (i = 0; i < MAX_RUNNING; i++){
#ifdef DEBUG
		fprintf(stderr, "Attempting to initialize pcb for P%d\n", i);
#endif
		initPcb(&pcbArr[i], i);
	}
}

// Returns a pcb to its state before it was assigned to a process
void resetPcb(PCB * pcb){
	setDefaults(pcb);

	if (pcb->currentQueue != NULL)
		removeFromCurrentQueue(pcb);
}

// Sets the logical address and type of the last memory reference
void setLastReferenceInPcb(PCB * pcb, int address, RefType type, 
			   Clock startTime){
	pcb->lastReference.address = address;
	pcb->lastReference.type = type;
	copyTime(&pcb->lastReference.startTime, startTime);
}

// Sets the time at which an I/O operation will complete
void setIoCompletionTimeInPcb(PCB * pcb, Clock completeTime){
	copyTime(&pcb->lastReference.pageCompleteTime, completeTime);
	pcb->lastReference.completionTimeIsSet = true;
}

// Adds access time to accululator and increments the number of accesses
void completeReferenceInPcb(PCB * pcb, Clock refCompletionTime){

	// Sets end time for last reference
	copyTime(&pcb->lastReference.endTime, refCompletionTime);

	// Adds elapsed time durring last reference to total
	Clock elapsed = clockDiff(pcb->lastReference.endTime,
				  pcb->lastReference.startTime);
	incrementClock(&pcb->totalAccessTime, elapsed);

	// Increments total number of references
	pcb->totalReferences += 1;

	// Indicates last end time is invalid
	pcb->lastReference.completionTimeIsSet = false;
}

// Returns effective memory access time
Clock getEatFromPcb(const PCB * pcb){
	return clockDiv(pcb->totalAccessTime, pcb->totalReferences);
}
