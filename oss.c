// oss.c was created by Mark Renard on 4/29/2020.
//
// This program simulates memory management.

#include "clock.h"
#include "getSharedMemoryPointers.h"
#include "logging.h"
#include "pcb.h"
#include "frameDescriptor.h"
#include "perrorExit.h"
#include "protectedClock.h"
#include "qMsg.h"
#include "queue.h"
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
static int getParsedMessage(int, int*, int*);
static void processTermination(int simPid);
static void processRead(int simPid, int msg, Queue * q);
static void processWrite(int simPid, int msg, Queue * q);
static void waitForProcess(pid_t realPid);
static void assignSignalHandlers();
static void cleanUpAndExit(int param);
static void cleanUp();

// Constants
static const Clock MIN_FORK_TIME = {MIN_FORK_TIME_SEC, MIN_FORK_TIME_NS};
static const Clock MAX_FORK_TIME = {MAX_FORK_TIME_SEC, MAX_FORK_TIME_NS};
static const Clock MAIN_LOOP_INCREMENT = {LOOP_INCREMENT_SEC,
					  LOOP_INCREMENT_NS};
static const struct timespec SLEEP = {0, 50000};

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
//	openLogFile();		// Opens file written to in logging.c

	srand(BASE_SEED - 1);   // Seeds pseudorandom number generator

	// Creates shared memory region and gets pointers
	getSharedMemoryPointers(&shm, &systemClock, &frameTable, &pcbs, 
				IPC_CREAT);

        // Creates message queues
        requestMqId = getMessageQueue(REQUEST_MQ_KEY, MQ_PERMS | IPC_CREAT);
        replyMqId = getMessageQueue(REPLY_MQ_KEY, MQ_PERMS | IPC_CREAT);

	openLogFile();
//	initStats();

	// Initializes system clock and shared arrays
	initPClock(systemClock);
	initPcbArray(pcbs);
	
	// Generates processes, grants requests, and resolves deadlock in a loop
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

	// Launches processes and resolves deadlock until limits reached
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
		while (getParsedMessage(requestMqId, &msg, &senderSimPid)){
			if (msg == TERMINATE){
				processTermination(senderSimPid);
				running--;
			}

			else if (msg < 0) processWrite(senderSimPid, ~msg, &q);
			else processRead(senderSimPid, msg, &q);
		}

		// Checks queue for requests 
//		checkQueue(q);

		// Increments system clock
		incrementPClock(systemClock, MAIN_LOOP_INCREMENT);

		i++;
	} while ((running > 0 || launched < MAX_LAUNCHED));// && i < 100); //launched < 50); //MAX_RUNNING);

}

// Forks & execs a user process with the assigned logical pid, returns child pid
static void launchUserProcess(){
	pid_t realPid;	// The real pid of the child process
	int simPid;	// The logical pid of the process

	// Forks, exiting on error
	if ((realPid = fork()) == -1){
		perrorExit("Failed to fork");
	}

	// Child process calls execl on the user program binary
	if (realPid == 0){

		// Assigns a free process control block to the child process
		if ((simPid = assignFreePcb(pcbs, realPid)) == -1)
			perrorExit("launchUserProcess called with no free pcb");

		// Converts simPid ot string
		char sPid[BUFF_SZ];
		sprintf(sPid, "%d", simPid);
		
		// Execs the child process
		execl(USER_PROG_PATH, USER_PROG_PATH, sPid, NULL);
		perrorExit("Failed to execl");
	}
}

// Checks message queue, returning 1 and retrieving message if one exists
static int getParsedMessage(int mqId, int * msg, int * senderSimPid){
	char msgBuff[BUFF_SZ];
	long int msgType;

	if(getMessage(mqId, msgBuff, &msgType)){
		*msg = atoi(msgBuff);
		*senderSimPid = (int) msgType - 1;

		fprintf(stderr, "Got msg %d from P%d\n", *msg, *senderSimPid);

		return 1;
	}

//	fprintf(stderr, "No message!\n");

	return 0;
}

static void processTermination(int simPid){
	fprintf(stderr, "oss processing termintation of P%d\n", simPid);

	logTermination(simPid, getPTime(systemClock));

	waitForProcess(pcbs[simPid].realPid);
	resetPcb(&pcbs[simPid]);	
}

static void processWrite(int simPid, int address, Queue * q){
	fprintf(stderr, "oss processing P%d write to %d\n", simPid, address);

	Clock now = getPTime(systemClock);
	logWriteRequest(simPid, address, now);
	logWriteGranted(address, 123, simPid, now);

	sendMessage(replyMqId, "\0", simPid + 1);
}

static void processRead(int simPid, int logicalAddress, Queue * q){
	fprintf(stderr, "oss processing P%d read from %d\n", simPid, 
		logicalAddress);

	Clock now;		// Storage for the current time

	int pageNum;		// Page number of requested address
	int offset;		// Offset of requested address
	PageTableEntry * page;	// Page corresponding to the address

//	int frameNumber;	// The physical frame number for the page
	int physicalAddress;	// The requested physical address

	// Logs request
	now = getPTime(systemClock);
	logReadRequest(simPid, logicalAddress, now);

	// Gets page number
	pageNum = logicalAddress / PAGE_SIZE;
	
	// Computes offset & gets page table entry for the logical address
	offset = logicalAddress % PAGE_SIZE;
	page = &pcbs[simPid].pageTable[pageNum];

	// Gets the corresponding physical address
	physicalAddress = page->frameNumber * PAGE_SIZE + offset;

	fprintf(stderr, "oss: logical: %d page: %d frame: %d physical: %d\n",
		logicalAddress, pageNum, page->frameNumber, physicalAddress);

	// Kills the process if the address is illegal
	if (pageNum >= pcbs[simPid].lengthRegister){
	//	killProcess(simPid);
		fprintf(stderr, "oss should kill P%d\n", simPid);
		return;
	}

	// Enqueues the request if the page is invalid
	if (!pcbs[simPid].pageTable[pageNum].valid) {
	//	enqueueRequest(simPid, logicalAddress, q);
		fprintf(stderr, "oss should enqueue P%d\n", simPid);
	//	return;
	}

	// Grants the request otherwise
//	grantRequest(simPid, logicalAddress);
	
	// Grants the request
//	grantRequest
	fprintf(stderr, "oss should grant the request for %d from P%d\n",
		logicalAddress, simPid);
	
	logReadGranted(logicalAddress, page->frameNumber, simPid, now);

	sendMessage(replyMqId, "\0", simPid + 1);
}

//static void grantRequest

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

