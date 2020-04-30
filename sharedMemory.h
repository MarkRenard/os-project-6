// sharedMemory.h was created by Mark Renard on 2/21/2020
// This file contains a headers for the functions sharedMemory and
// removeSegment, which are defined in sharedMemory.c

#ifndef SHAREDMEMORY_H
#define SHAREDMEMORY_H

#include <sys/ipc.h>
#include <sys/shm.h>

char * sharedMemory(int size, int mask);
void removeSegment();
void detach(char * shm);
void initializeSharedMemory(char * shm, int bufferSize, char byte);
void printSharedMemory(char * shm, int size);

#endif
