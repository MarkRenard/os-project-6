// userProgram.c was created by Mark Renard on 4/12/2020
//
// This program sends messages to an operating system simulator, simulating a
// process that requests and relinquishes resources at random times.

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "constants.h"
#include "frameDescriptor.h"
#include "getSharedMemoryPointers.h"
#include "pcb.h"
#include "perrorExit.h"
#include "protectedClock.h"
#include "qMsg.h"
#include "randomGen.h"
#include "sharedMemory.h"

// Prototypes
static void simulateMemoryReferencing();
static int getAddress();
static void makeReadReference(int address);
static void makeWriteReference(int address);
static void signalTermination();

// Constants
static const Clock MIN_REF_INTERVAL = {MIN_REF_INTERVAL_SEC, 
				       MIN_REF_INTERVAL_NS};
static const Clock MAX_REF_INTERVAL = {MAX_REF_INTERVAL_SEC, 
				       MAX_REF_INTERVAL_NS};
static const Clock CLOCK_UPDATE = {CLOCK_UPDATE_SEC, CLOCK_UPDATE_NS};

// Static global variables
static char * shm;                              // Pointer to shared memory
static ProtectedClock * systemClock;            // Shared memory system clock
static FrameDescriptor * frameTable;            // Shared memory frame table
static PCB * pcbs;                              // Shared process control blocks

static int simPid;	// Logical pid of the process
static int weighted;	// Whether the random address selection is weighted
static int requestMqId; // Id of message queue for resource requests & release
static int replyMqId;   // Id of message queue for replies from oss

int main(int argc, char * argv[]){
	exeName = argv[0];		// Sets exeName for perrorExit
	simPid = atoi(argv[1]);		// Gets process's logical pid
	weighted = atoi(argv[2]);	// Gets flag for address weighting
	srand(BASE_SEED + simPid); 	// Seeds pseudorandom number generator

	// Attaches to shared memory and gets pointers
	getSharedMemoryPointers(&shm, &systemClock, &frameTable, &pcbs, 0);

	// Gets message queues
        requestMqId = getMessageQueue(REQUEST_MQ_KEY, MQ_PERMS);
        replyMqId = getMessageQueue(REPLY_MQ_KEY, MQ_PERMS);

	simulateMemoryReferencing();

	// Prepares to exit
	signalTermination();
	detach(shm);

	return 0;
}

// Repeatedly sents requests for memory references to oss
static void simulateMemoryReferencing(){
	Clock now = {0, 0};		// Storage for the currnet time
	Clock referenceTime = {0, 0};	// Time at which to make a reference
	int maxReferences; 		// References before termination chance
	int numReferences = 0;		// References made since reset

	// Randomly determines number of references (900 to 1100 by default)
	maxReferences = randInt(MIN_REFERENCES, MAX_REFERENCES);

	// Repeatedly makes read or write references and terminates
	while (numReferences < maxReferences \
	       || !randBinary(TERMINATION_PROBABILITY)) {

		now = getPTime(systemClock);
	
		// Makes a reference at or after reference time
		if (clockCompare(now, referenceTime) >= 0){

			// Updates numReferences
			numReferences = (numReferences + 1) \
					% (maxReferences + 1);

			// Updates reference time
			copyTime(&referenceTime, now);
			incrementClock(&referenceTime, 
					randomTime(MIN_REF_INTERVAL, 
						   MAX_REF_INTERVAL));

			// Makes read or write reference 
			if (randBinary(READ_PROBABILITY)){
				makeReadReference(getAddress());
			} else {
				makeWriteReference(getAddress());
			}

			// Increments the protected system clock
			incrementPClock(systemClock, CLOCK_UPDATE);

			// Waits for reference to finish
			waitForMessage(replyMqId, NULL, simPid + 1);
		}
	}
}

// Sends a request to oss to read from memory at a logical address
static void makeReadReference(int address){

	char addr[BUFF_SZ];
	sprintf(addr, "%d", address);

	fprintf(stderr, "\n\t\tP%d READING %s\n\n", simPid, addr);

	sendMessage(requestMqId, addr, simPid + 1);
}

// Sends a request to oss to write to memory at a logical address
static void makeWriteReference(int address){
	char addr[BUFF_SZ];
	sprintf(addr, "%d", ~address);

	fprintf(stderr, "\n\t\tP%d WRITING %s\n\n", simPid, addr);

	sendMessage(requestMqId, addr, simPid + 1);
}

// Returns a reference to an address in memory allocated to the process
static int getAddress(){
	return randInt(0, pcbs[simPid].lengthRegister * PAGE_SIZE - 1);
}

static void signalTermination(){
	char msgBuff[BUFF_SZ];
	sprintf(msgBuff, "%d", TERMINATE);

	fprintf(stderr, "\n\t\tP%d TERMINATING\n\n", simPid);

	sendMessage(requestMqId, msgBuff, simPid + 1);
}

/*
// Sends a message over a message queue requesting random resources
static bool requestResources( int simPid){
	char msgBuff[BUFF_SZ];	// Message buffer

	// Endcodes logical address
		

	// Sends the message
	sprintf(msgBuff, "%d", encoded);
	sendMessage(requestMqId, msgBuff, simPid + 1);

	return true;

}

// Sends a message over a message queue releasing random resources
static bool releaseResources(ResourceDescriptor * resources,
			     Message * messages, int simPid){
	char msgBuff[BUFF_SZ];	// Message buffer
	int rNum;		// Resource index
	int quantity;		// Actual quantity requested
	int encoded;		// Encoded message
	
	// Selects a held resource at random or returns if no resources held
	if ((rNum = getRandomRNum()) == -1){

		 return false;
	}

	// Randomly determines quantity to release
	quantity = randInt(1, targetHeld[rNum]);

	// Records new target
	targetHeld[rNum] -= quantity;

	// Encodes resource index and quantity in a message
	encoded = -((MAX_INST + 1) * rNum + quantity);

	// Sends the message
	sprintf(msgBuff, "%d", encoded);
	sendMessage(requestMqId, msgBuff, simPid + 1);

	return true;

}

// Gets a randomly chosen index of a held resource or -1 if no resources held
static int getRandomRNum(){
	int resourceCount;		// Number of resource classes held
	int resInd[NUM_RESOURCES]; 	// Indices of held resources

	// Records the indecies of held resources in resInd
	int i = 0, j = 0;
	for ( ; i < NUM_RESOURCES; i++)
		if (targetHeld[i] > 0) resInd[j++] = i;
	resourceCount = j;

	// Returns -1 if no resoures are held
	if (resourceCount == 0) return -1;

	// Returns the index of the randomly chosen resource
	return resInd[randInt(0, resourceCount - 1)];
}
*/
