// getSharedMemoryPointers.h was created by Mark Renard on 3/27/2020 and
// modified on 4/30/2020.
//
// This file contains a header for the function getSharedMemoryPointers to be
// used in assignment 6.

#ifndef GETSHAREDMEMORYPOINTERS_H
#define GETSHAREDMEMORYPOINTERS_H

#include "constants.h"
#include "pcb.h"
#include "protectedClock.h"
#include "frameDescriptor.h"
#include "sharedMemory.h"

int getSharedMemoryPointers(char ** shm,  ProtectedClock ** systemClock,
                            FrameDescriptor ** frameTable, PCB ** pcbs, int flags);

#endif
