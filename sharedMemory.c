// sharedMemory.c was created by Mark Renard on 2/21/2020
//
// This file contains an implementation of a function that returns a pointer
// to a shared memory region of the requested size in bytes corresponding to
// the key set in shmkey.h. If one does not exist and mask is set equal to
// IPC_CREAT as defined in sys/ipc.h, one will be created.

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "perrorExit.h"
#include "shmkey.h"

static int shmid; // The shmid of the shared memory region

// Returns a pointer to a new shared memory region
char * sharedMemory(int size, int mask){
	shmid = shmget ( SHMKEY, size, 0600 | mask );

	// Prints error message and exits if unsuccessful
	if (shmid == -1)
		 perrorExit("sharedMemory call to shmget");

	// Returns pointer to shared memory region
	return shmat (shmid, 0, 0);
}

// Detatches the process from shm or exits with error message on failure
void detach(char * shm){
	if(shmdt(shm) == -1) perrorExit("Failed to detach");
}

// Removes a shared memory segment previously created with sharedMemory
void removeSegment(){
	if (shmctl(shmid, IPC_RMID, NULL) == -1)
		perrorExit("removeSegment failed");
}

// Sets each byte in the shared memory region to the value of the byte parameter
void initializeSharedMemory(char * shm, int bufferSize, char byte){
	int i;
	for (i = 0; i < bufferSize; i++){
		shm[i] = byte;
	}
}

// Prints the contents of shared memory
void printSharedMemory(char * shm, int size){
	int numInts = size / sizeof(int);

	int i, j = 0;
	for (i = 0; i < numInts; i++){
		fprintf(stderr, "%c", ((int*)shm)[i]);
		if (++j == 80) fprintf(stderr, "\n");
	}
}	
