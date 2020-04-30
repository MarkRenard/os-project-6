// pcb.c was created by Mark Renard on 4/11/2020 as message.c and modified on
// 4/29/2020.
//
// This file contains functions for initializing and resetting process control
// block structures that contain page tables.

#include "constants.h"
#include "queue.h"
#include "pcb.h"

#include <stdio.h>

// Sets non-queue values to defaults
static void setDefaults(PCB * pcb){
	pcb->realPid = -1;

	int i = 0;
	for( ; i < MAX_ALLOC_PAGES; i++){
		pcb->pageTable[i].valid = 0;
		pcb->pageTable[i].dirty = 0;
		pcb->pageTable[i].frameNumber = -1;
	}

	pcb->lengthRegister = 0;
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
