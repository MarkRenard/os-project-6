// getSharedMemoryPointers.c was created by Mark Renard on 3/27/2020 and 
// modified on 4/29/2020.
//
// This file contains the definition of a shared memory function specific to
// assignment 6. This function is used by oss.c and userProgram.c.

#include "clock.h"
#include "constants.h"
#include "pcb.h"
#include "protectedClock.h"
#include "frameDescriptor.h"
#include "sharedMemory.h"

int getSharedMemoryPointers(char ** shm,  ProtectedClock ** systemClock,
			     FrameDescriptor ** frameTable,
			     PCB ** pcbs, int ** weights, int flags) {

	// Computes size of the shared memory region
	int shmSize = sizeof(ProtectedClock) \
		      + sizeof(FrameDescriptor) * NUM_FRAMES \
                      + sizeof(PCB) * MAX_RUNNING \
		      + sizeof(int) * MAX_ALLOC_PAGES;

 	// Attaches to shared memory
        *shm = sharedMemory(shmSize, flags);

	// Gets pointer to simulated system clock
	*systemClock = (ProtectedClock *)(*shm);

	// Gets pointer to first resource descriptor
	*frameTable = (FrameDescriptor *)(*shm + sizeof(ProtectedClock));

	// Gets pointer to message array
	*pcbs = (PCB *)( ((char*)(*frameTable)) \
		     + (sizeof(FrameDescriptor) * NUM_FRAMES));

	// Gets pointer to array of weights
	*weights = (int *)(*pcbs + MAX_RUNNING);

	return shmSize;
}

