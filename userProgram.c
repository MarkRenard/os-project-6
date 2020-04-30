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
//static void signalTermination(int simPid);
//static bool requestResources(ResourceDescriptor *, Message *, int);
//static bool releaseResources(ResourceDescriptor *, Message *, int);
//static int getRandomRNum();
//static bool aSecondHasPassed(Clock now, Clock startTime);
/*
// Constants
static const Clock MIN_CHECK = {MIN_CHECK_SEC, MIN_CHECK_NS};
static const Clock MAX_CHECK = {MAX_CHECK_SEC, MAX_CHECK_NS};
static const Clock MIN_RUN_TIME = {MIN_RUN_TIME_SEC, MIN_RUN_TIME_NS};
static const Clock CLOCK_UPDATE = {CLOCK_UPDATE_SEC, CLOCK_UPDATE_NS};
*/
// Static global variables
static char * shm;                              // Pointer to shared memory
static ProtectedClock * systemClock;            // Shared memory system clock
static FrameDescriptor * frameTable;            // Shared memory frame table
static PCB * pcbs;                              // Shared process control blocks

static int requestMqId; // Id of message queue for resource requests & release
static int replyMqId;   // Id of message queue for replies from oss

int main(int argc, char * argv[]){
	exeName = argv[0];		// Sets exeName for perrorExit
	int simPid = atoi(argv[1]);	// Gets process's logical pid
	srand(BASE_SEED + simPid); 	// Seeds pseudorandom number generator
/*
        ProtectedClock * systemClock;	// Shared memory system clock
        ResourceDescriptor * resources;	// Shared memory resource table
        Message * messages;		// Shared memory message vector

	Clock decisionTime;		// Time to request, relese, or terminate
	Clock startTime;		// Time the process started
	Clock now;			// Temp storage for time
*/
	// Attatches to shared memory and gets pointers
	getSharedMemoryPointers(&shm, &systemClock, &frameTable, &pcbs, 0);

	// Initializes clocks
/*
	startTime = getPTime(systemClock);
	decisionTime = startTime;
*/
	// Gets message queues
        requestMqId = getMessageQueue(REQUEST_MQ_KEY, MQ_PERMS | IPC_CREAT);
        replyMqId = getMessageQueue(REPLY_MQ_KEY, MQ_PERMS | IPC_CREAT);
//	char reply[BUFF_SZ];
/*
	// Repeatedly requests or releases resources or terminates
	bool terminating = false;
	bool msgSent = false;	
	while (!terminating) {

		// Decides when current time is at or after decision time
		now = getPTime(systemClock);
		if (clockCompare(now, decisionTime) >= 0){

			// Updates decision time
			incrementClock(&decisionTime, 
					randomTime(MIN_CHECK, MAX_CHECK));

			// Decides whether to terminate
			if (aSecondHasPassed(now, startTime) \
			    && randBinary(TERMINATION_PROBABILITY)){
				signalTermination(simPid);
				terminating = true;
				msgSent = true;

			// Decides whether to request or release resources
			} else if (randBinary(REQUEST_PROBABILITY)){
				msgSent = requestResources(resources, messages,
							   simPid);
			} else {
				msgSent = releaseResources(resources, messages,
							   simPid);
			}

			// Increments the protected system clock
			incrementPClock(systemClock, CLOCK_UPDATE);
		}

		// Waits for response to request
		if (msgSent){
			msgSent = false;

			waitForMessage(replyMqId, reply, simPid + 1);

			if (strcmp(reply, KILL_MSG) == 0){

				terminating = true;
			}
		}
	}
*/
	// Prepares to exit
	detach(shm);

	return 0;
}
/*
static void signalTermination(int simPid){
	char msgBuff[BUFF_SZ];
	sprintf(msgBuff, "0");
	sendMessage(requestMqId, msgBuff, simPid + 1);
}

// Sends a message over a message queue requesting random resources
static bool requestResources(ResourceDescriptor * resources, 
			     Message * messages, int simPid){
	char msgBuff[BUFF_SZ];	// Message buffer
	int rNum;		// Resource index
	int maxRequest;		// Max quantity of requested resources
	int quantity;		// Actual quantity requested
	int encoded;		// Encoded message

	// Randomly selects a resource to request
	rNum = randInt(0, NUM_RESOURCES - 1);

	// Computes maximum request
	maxRequest = resources[rNum].numInstances - targetHeld[rNum];

	// Returns if none can be requested
	if (maxRequest == 0) return false;

	// Randomly selects quantity to request
	quantity = randInt(1, maxRequest);

	// Records new target
	targetHeld[rNum] += quantity;

	// Endcodes resource index and quantity in a message
	encoded = (MAX_INST + 1) * rNum + quantity;

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
	int resInd[NUM_RESOURCES]; // Indices of held resources

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

static bool aSecondHasPassed(Clock now, Clock startTime){
	static bool hasPassed = false;

	if (!hasPassed){
		Clock diff = clockDiff(now, startTime);
		hasPassed = (clockCompare(diff, MIN_RUN_TIME) >= 0);
	}

	return hasPassed;
}	
*/
