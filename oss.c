// oss.c was created by Mark Renard on 4/29/2020.
//
// This program simulates memory management.

#include "bitVector.h"
#include "clock.h"
#include "getSharedMemoryPointers.h"
#include "logging.h"
#include "pcb.h"
#include "frameDescriptor.h"
#include "perrorExit.h"
#include "protectedClock.h"
#include "qMsg.h"
#include "queue.h"
#include "randomGen.h"
//#include "stats.h"

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

// Prototypes
static void simulateMemoryManagement();
static void launchUserProcess();
static int messageReceived(int*, int*);
static void processTermination(int simPid);
static void deallocateFrames(PCB * pcb);
static void processReference(int simPid, Queue * q);
static void checkPagingQueue(Queue * q);
static void allocateFrame(int frameNum, PCB * pcb);
static void deallocateFrame(int frameNum);
static int selectVictim();
static void grantRequest(int simPid);
static void waitForProcess(pid_t realPid);
static void assignSignalHandlers();
static void cleanUpAndExit(int param);
static void cleanUp();

// Constants
static const Clock MIN_FORK_TIME = {MIN_FORK_TIME_SEC, MIN_FORK_TIME_NS};
static const Clock MAX_FORK_TIME = {MAX_FORK_TIME_SEC, MAX_FORK_TIME_NS};

//static const Clock MAIN_LOOP_INCREMENT = {LOOP_INCREMENT_SEC,
//					  LOOP_INCREMENT_NS};

static const Clock IO_OP_TIME = {IO_OPERATION_SEC, IO_OPERATION_NS};
static const Clock MEM_ACCESS_TIME = {MEM_ACCESS_SEC, MEM_ACCESS_NS};

// static const struct timespec SLEEP = {0, 50000};

// Static global variables
static char * shm;			// Pointer to shared memory
static ProtectedClock * systemClock;	// Shared memory system clock
static FrameDescriptor * frameTable;	// Shared memory frame table
static PCB * pcbs;			// Shared process control blocks

static int requestMqId;	// Id of message queue for resource requests & release
static int replyMqId;	// Id of message queue for replies from oss

int main(int argc, char * argv[]){
	exeName = argv[0];	// Assigns exeName for perrorExit
	assignSignalHandlers(); // Sets response to ctrl + C & alarm
	openLogFile();		// Opens file written to in logging.c

	srand(BASE_SEED - 1);   // Seeds pseudorandom number generator

	// Creates shared memory region and gets pointers
	getSharedMemoryPointers(&shm, &systemClock, &frameTable, &pcbs, 
				IPC_CREAT);

        // Creates message queues
        requestMqId = getMessageQueue(REQUEST_MQ_KEY, MQ_PERMS | IPC_CREAT);
        replyMqId = getMessageQueue(REPLY_MQ_KEY, MQ_PERMS | IPC_CREAT);

//	initStats();

	// Initializes system clock and shared arrays
	initPClock(systemClock);
	initPcbArray(pcbs);
	
	// Generates processes, performs paging, and 
	simulateMemoryManagement();

//	logStats();

	cleanUp();

	return 0;
}

// Generates processes, grants requests, and resolves deadlock in a loop
void simulateMemoryManagement(){
	Clock timeToFork = zeroClock();	// Time to launch user process 
	Queue  q;			// Queue of requesting processes

	int running = 0;		// Currently running child count
	int launched = 0;		// Total children launched
	int msg;			// Int representation of a msg
	int senderSimPid;		// simPid of message sender

	initializeQueue(&q);

	int i = 0;

	// Launches processes, grants or enqueues requests, allocates pages
	do {

		// Launches user processes at random times if within limits
		if (clockCompare(getPTime(systemClock), timeToFork) >= 0){
			 
			// Launches process if within limits
			if (running < MAX_RUNNING && launched < MAX_LAUNCHED){
				launchUserProcess();

				running++;
				launched++;
			}

			// Selects new random time to launch a new user process
			incrementClock(&timeToFork, randomTime(MIN_FORK_TIME,
							       MAX_FORK_TIME));
		}

		// Checks message queue for messages
		while (messageReceived(&senderSimPid, &msg)){

			// If process terminated, waits for it and frees memory
			if (msg == TERMINATE){
				processTermination(senderSimPid);
				running--;
			}

			// Grants or enqueues request for memory reference
			else processReference(senderSimPid, &q);
		}

		// Increments system clock when all processes are waiting
		if (q.count == running)
			incrementPClock(systemClock, IO_OP_TIME);

		// Performs the clock replacement algorithm 
		checkPagingQueue(&q);

		i++;
	} while ((running > 0 || launched < MAX_LAUNCHED));// && i < 100); //launched < 50); //MAX_RUNNING);

}

// Forks & execs a user process with the assigned logical pid, returns child pid
static void launchUserProcess(){
	pid_t realPid;	// The real pid of the child process
	int simPid;	// The logical pid of the process

	// Gets the index of a pcb without a real pid assigned to it
	if ((simPid = getFreePcbIndex(pcbs)) == -1)
		perrorExit("launchUserProcess called with no free pcb");

	// Forks, exiting on error
	if ((realPid = fork()) == -1)
		perrorExit("Failed to fork");

	// Child process calls execl on the user program binary
	if (realPid == 0){

		// Converts simPid to string
		char sPid[BUFF_SZ];
		sprintf(sPid, "%d", simPid);
		
		// Execs the child process
		execl(USER_PROG_PATH, USER_PROG_PATH, sPid, NULL);
		perrorExit("Failed to execl");
	}

	// Assigns realPid to selected pcb in parent
	pcbs[simPid].realPid = realPid;

}

// Checks message queue, returning 1 and parsing message to pcb if one exists
static int messageReceived(int * senderSimPid, int * msg){
	char msgBuff[BUFF_SZ];	// Buffer for storing the message
	long int msgType;	// Storage for the message type

	int address;	// Referenced address
	RefType type;	// Referenced type

	// Parses a message from the request message queue
	if(getMessage(requestMqId, msgBuff, &msgType)){

		// Converts message queue values
		*msg = atoi(msgBuff);
		*senderSimPid = (int) msgType - 1;

		// Returns immediately if the process terminated
		if (*msg == TERMINATE) return 1;

		// Decodes the address and request type
		address = (*msg < 0 ? ~*msg : *msg);
		type = (*msg < 0 ? WRITE_REFERENCE : READ_REFERENCE);

		// Sets the last reference in the sender pcb
		setLastReferenceInPcb(&pcbs[*senderSimPid], address, type, 
				      getPTime(systemClock));

		fprintf(stderr, "Got msg %d from P%d\n", *msg, *senderSimPid);

		return 1;
	}

	return 0; // Returns 0 if no message was received
}

// Logs termination, waits for terminated process, and deallocates frames
static void processTermination(int simPid){
	fprintf(stderr, "oss processing termintation of P%d\n", simPid);
	logTermination(simPid, getPTime(systemClock), &pcbs[simPid]);
	waitForProcess(pcbs[simPid].realPid);
	deallocateFrames(&pcbs[simPid]);
	resetPcb(&pcbs[simPid]);	
}

// Deallocates all of the frames allocated to a process
static void deallocateFrames(PCB * pcb){
	int i;
	for (i = 0; i < pcb->lengthRegister; i++){
		deallocateFrame(pcb->pageTable[i].frameNumber);
	}
}

// Checks the validity of a reference and grants it or enqueues or kills process
static void processReference(int simPid, Queue * q){
	Reference ref;		// The memory reference to process
	int pageNum;		// Page number of requested address

	// Gets the reference to process
	ref = pcbs[simPid].lastReference;

	// Gets page number
	pageNum = ref.address / PAGE_SIZE;

	// Logs request
	logRequest(simPid, ref, getPTime(systemClock));


	// Kills the process if the address is illegal
	if (pageNum >= pcbs[simPid].lengthRegister){
	//	killProcess(simPid);
		fprintf(stderr, "oss should kill P%d\n", simPid);
		return;
	}

	// Enqueues the request if the page is invalid
	if (!pcbs[simPid].pageTable[pageNum].valid) {
		logPageFault(ref.address);
		enqueue(q, &pcbs[simPid]);
		fprintf(stderr, "oss enqueued P%d\n", simPid);
		return;
	}

	// Grants and logs the request otherwise
	grantRequest(simPid);
	logGrantedRequest(ref, pcbs[simPid].pageTable[pageNum].frameNumber,
			  simPid, getPTime(systemClock));

/*
	if (pcbs[simPid].lastReference.type == READ_REFERENCE){
		logReadGranted(logicalAddress, 123 *page->frameNumber*, 
		simPid, now);
		fprintf(stderr, "granting read request for %d from P%d\n",
			logicalAddress, simPid);
	} else {
		logWriteGranted(logicalAddress, 123 * page->frameNumber*, 
		simPid, now);
		fprintf(stderr, "granting write request for %d from P%d\n",
			logicalAddress, simPid);
	}
*/

}

// Performs the clock replacement algorithm on queued memory references 
static void checkPagingQueue(Queue * q){
//	int freeFrameNum;	// Unallocated frame number
//	int victimFrameNum;	// Frame number of frame to swap out
	Clock completionTime;	// Time at which I/O will complete
	int frameNum;		// Number of frame to reallocate

	// Checks the progress of I/O if a frame was read or written
	if (q->front != NULL && q->front->lastReference.endTimeIsSet) {

		fprintf(stderr, "\t\tEND TIME SET FOR P%d!!!!!!\n", 
			q->front->simPid);

		// Returns if reference end time is in the future
		if (clockCompare(q->front->lastReference.endTime, 
				 getPTime(systemClock)) > 0){
			fprintf(stderr, "\t\tEND TIME NOT ARRIVED\n");
			return;
		}

		// Completes memory reference if read/write has completed
		else {
			grantRequest(q->front->simPid);
			q->front->lastReference.endTimeIsSet = false;
			logGrantedQueuedRequest(q->front->simPid, 
						q->front->lastReference);
			dequeue(q);
		}
	}

	if (q->count > 0){

		// Sets time swap will complete
		copyTime(&completionTime, getPTime(systemClock));
		incrementClock(&completionTime, IO_OP_TIME);
		
		// Gets available frame number or selects a victim frame
		if ((frameNum = getIntFromBitVector()) == -1){
			frameNum = selectVictim();
		
			// Logs the swap event
			logSwap(frameNum, q->front->simPid,
			        q->front->lastReference.address / PAGE_SIZE);

			// Adds time to write frame if it is dirty
			if (frameTable[frameNum].dirty){
				logDirty(frameNum);
				incrementClock(&completionTime, IO_OP_TIME);
			}

			// Sets completion time and deallocates frame
			setIoCompletionTimeInPcb(q->front, completionTime);
			deallocateFrame(frameNum);
		}
	
		// Allocates the frame to the front of the queue
		allocateFrame(frameNum, q->front);
	}

/*
	// Grants request at head while a frame is free
	while (q->count > 0 && (freeFrameNum = getIntFromBitVector()) != -1){
		allocateFrame(freeFrameNum, q->front);
		grantRequest(q->front->simPid);
		logGrantedQueuedRequest(q->front->simPid, 
					q->front->lastReference);
		dequeue(q);
	}
	
	// If all frames taken and queue not empty, selects victim & begins swap
	if (q->count > 0) {

		// Uses clock algorithm to select victim frame number, logs swap
		victimFrameNum = selectVictim();
		logSwapping(victimFrameNum, q->front->simPid,
			    q->front->lastReference.address / PAGE_SIZE);

		// Starts I/O and sets completion time
		copyTime(&completionTime, getPTime(systemClock));
		incrementClock(&completionTime, IO_OP_TIME);
		if (frameTable[victimFrameNum].dirty){

			// Doubles completion time if write to disk is necessary
			logDirty(victimFrameNum);
			incrementClock(&completionTime, IO_OP_TIME);
		}
		setIoCompletionTimeInPcb(q->front, completionTime);

		// Re-allocates frame to process at front of queue
		deallocateFrame(victimFrameNum);
		allocateFrame(victimFrameNum, q->front);
	}
*/
}

// Allocates a frame to a process
static void allocateFrame(int frameNum, PCB * pcb){
	int pageNum = pcb->lastReference.address / PAGE_SIZE;

	// Updates bit vector
	reserveInBitVector(frameNum);

	// Updates page table
	pcb->pageTable[pageNum].frameNumber = frameNum;
	pcb->pageTable[pageNum].valid = 1;
	pcb->pageTable[pageNum].dirty = 0;

	// Updates frame table
	frameTable[frameNum].simPid = pcb->simPid;
	frameTable[frameNum].pageNum = pageNum;
	frameTable[frameNum].reference = 1;
	frameTable[frameNum].dirty = 0;
}

// Deallocates a frame from a process
static void deallocateFrame(int frameNum){

	// updates bit vector
	freeInBitVector(frameNum);

	// Gets process and page indices from frame descriptor
	int simPid = frameTable[frameNum].simPid;
	int pageNum = frameTable[frameNum].pageNum;

	// Deallocates frame in page table
	if (simPid != EMPTY)
		pcbs[simPid].pageTable[pageNum].valid = 0;

	// Deallocates frame in frame table
	frameTable[frameNum].simPid = EMPTY;

/*
	if (frameTable[frameNum].simPid != EMPTY){
		int simPid = frameTable[frameNum].simPid;
		pcbs[simPid].pageTable[frameNum
	}

	frameTable[frameNum].simPid
*/
}

// Returns the frame number of a victim frame
static int selectVictim(){

	// Random selection for now
	return randInt(0, NUM_FRAMES - 1);
}

// Increments clock and sets reference and dirty bit if the operation was a write 
static void grantRequest(int simPid){
	int logicalAddress;	// The requested logical address
	int pageNum;		// Page number of requested address
	int offset;		// Offset of requested address
	PageTableEntry * page;	// Page corresponding to the address
	int physicalAddress;	// The requested physical address
	
	// Gets requested logical address and computes page number and offset
	logicalAddress = pcbs[simPid].lastReference.address;
	pageNum = logicalAddress / PAGE_SIZE;
	offset = logicalAddress % PAGE_SIZE;
	page = &pcbs[simPid].pageTable[pageNum];

	// Computes frame number and physical address
	physicalAddress = page->frameNumber * PAGE_SIZE + offset;

	// Sets dirty bits if operation was write operation
	if (pcbs[simPid].lastReference.type == WRITE_REFERENCE){
		frameTable[page->frameNumber].dirty = 1;
		page->dirty = 1;
	}

	// Sets reference
	frameTable[page->frameNumber].reference = 1;

	// Increments clock
	incrementPClock(systemClock, MEM_ACCESS_TIME);

	// Resets reference in pcb
	completeReferenceInPcb(&pcbs[simPid], getPTime(systemClock));

	// Sends reply message
	sendMessage(replyMqId, "\0", simPid + 1);
	fprintf(stderr, "oss: logical: %d page: %d frame: %d physical: %d\n",
		logicalAddress, pageNum, page->frameNumber, physicalAddress);
}

// Waits for the process with pid equal to the realPid parameter
static void waitForProcess(pid_t realPid){
        pid_t retval;
        while(((retval = waitpid(realPid, NULL, 0)) == -1)
                 && errno == EINTR);

        if (errno == ECHILD)
                perrorExit("waited for non-existent child");

}

// Determines the processes response to ctrl + c or alarm
static void assignSignalHandlers(){
	struct sigaction sigact;

	// Initializes sigaction values
	sigact.sa_handler = cleanUpAndExit;
	sigact.sa_flags = 0;

	// Assigns signals to sigact
	if ((sigemptyset(&sigact.sa_mask) == -1)
	    ||(sigaction(SIGALRM, &sigact, NULL) == -1)
	    ||(sigaction(SIGINT, &sigact, NULL)  == -1)){

		// Prints error message and exits on failure
		char buff[BUFF_SZ];
		sprintf(buff, "%s: Error: Failed to install signal handlers",
			exeName);
		perror(buff);
		exit(1);
	}
}

// Signal handler - closes files, removes shm, terminates children, and exits
static void cleanUpAndExit(int param){

	// Closes files, removes shm, terminates children
	cleanUp();

	// Prints error message
	char buff[BUFF_SZ];
	sprintf(buff,
		 "%s: Error: Terminating after receiving a signal",
		 exeName
	);
	perror(buff);

	// Exits
	exit(1);
}

// Kills child processes, closes message queues & files, removes shared mem
static void cleanUp(){

	// Handles multiple interrupts by ignoring until exit
	signal(SIGALRM, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);

	// Kills all other processes in the same process group
	kill(0, SIGQUIT);

	// Destroys semaphore protecting system clock
	while (pthread_mutex_destroy(&systemClock->sem) != 0 && errno == EBUSY);
	if (errno == EINVAL){
		char buff[BUFF_SZ];
		sprintf(buff, "%s: Error: ", exeName);
		perror("Attempted to destroy invalid semaphore");
	}

	// Removes message queues
	removeMessageQueue(requestMqId);
	removeMessageQueue(replyMqId);

	// Closes log file
	closeLogFile();

	// Detatches from and removes shared memory
	detach(shm);
	removeSegment();
}

