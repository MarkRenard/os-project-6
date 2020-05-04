// logging.c was created by Mark Renard on 4/13/2020.
//
// This file contains definitions for functions that aid in the collection,
// formatting, and logging of data pertinent to Assignment 6.

#include "clock.h"
#include "constants.h"
#include "pcb.h"
#include "perrorExit.h"
#include <stdio.h>

static FILE * log = NULL;
static int lines = 0;

// Opens the log file with name LOG_FILE_NAME or exits with an error message
void openLogFile(){
	if ((log = fopen(LOG_FILE_NAME, "w+")) == NULL)
		perrorExit("logging.c - failed to open log file");
}

// Closes the log file
void closeLogFile(){
	if (log != NULL){
		if (fclose(log) == -1)
			perrorExit("logging.c - error closing log file");
	}
}

// Logs when a process has terminated
void logTermination(int simPid, Clock time, const PCB * pcb){
	lines += 2;
	if (lines > MAX_LOG_LINES) return;

	Clock eat = getEatFromPcb(pcb);

	fprintf(log, "Master: P%d has terminated at time %03d : %09d\n",
		simPid, time.seconds, time.nanoseconds);

	fprintf(log, "\t\t Effective memory access time: %03d : %09d\n",
		eat.seconds, eat.nanoseconds);

	
}

// Logs a request to read from an address at a particular time
void logReadRequest(int simPid, int address, Clock time){
	if (++lines > MAX_LOG_LINES) return;

	fprintf(log, "Master: P%d requesting read of address %d at time "
		"%03d : %09d\n", simPid, address, time.seconds, 
		time.nanoseconds);
}

// Logs the granting of a request to read from an address
void logReadGranted(int address, int frameNum, int simPid, Clock time){
	if (++lines > MAX_LOG_LINES) return;

	fprintf(log, "Master: Address %d in frame %d, giving data to P%d at " \
		"time %03d : %09d\n", address, frameNum, simPid, time.seconds,
		time.nanoseconds);
}

// Logs a request to write to an address at a particular time
void logWriteRequest(int simPid, int address, Clock time){
	if (++lines > MAX_LOG_LINES) return;

	fprintf(log, "Master: P%d requesting write of address %d at time " \
		"%03d : %09d\n", simPid, address, time.seconds, 
		time.nanoseconds);

}

// Logs the granting of a request to write to an address
void logWriteGranted(int address, int frameNum, int simPid, Clock time){
	if (++lines > MAX_LOG_LINES) return;

	fprintf(log, "Master: Address %d in frame %d, writing data from P%d " \
		"to frame at time %03d : %09d\n", address, frameNum, simPid, 
		time.seconds, time.nanoseconds);
}

// Logs when a request was received by oss
void logRequest(int simPid, Reference ref, Clock time){
        fprintf(stderr, "oss processing P%d reference to %d\n", simPid,
                ref.address);

	if (ref.type == READ_REFERENCE)
		logReadRequest(simPid, ref.address, time);
	else
		logWriteRequest(simPid, ref.address, time);
}

// Logs when a request is immediately granted by oss
void logGrantedRequest(Reference ref, unsigned char frameNum, int simPid,
		       Clock time){

	if (ref.type == READ_REFERENCE){
		fprintf(stderr, "oss granting read request for %d from P%d",
			ref.address, simPid);
		logReadGranted(ref.address, frameNum, simPid, time);
	} else {
		fprintf(stderr, "oss granting write request for %d from P%d",
			ref.address, simPid);
		logWriteGranted(ref.address, frameNum, simPid, time);
	}
}

// Logs a page fault event
void logPageFault(int address){
	if (++lines > MAX_LOG_LINES) return;

	fprintf(log, "Master: Address %d is not in a frame, pagefault\n", 
		address);

}

// Logs swapping in of a page
void logSwap(int frameNum, int simPid, int pageNum){
	if (++lines > MAX_LOG_LINES) return;

	fprintf(log, "Master: Clearing frame %d and swapping in P%d page %d\n",
		frameNum, simPid, pageNum);	
}

// Logs that a frame was dirty
void logDirty(int frameNum){
	if (++lines > MAX_LOG_LINES) return;
	
	fprintf(log, "Master: Dirty bit of frame %d was set, adding " \
		"additional time to the clock\n", frameNum);
}

// Logs that a queued read request fulfillment was indicated to a process
void logReadIndication(int simPid, int address){
	if (++lines > MAX_LOG_LINES) return;

	fprintf(log, "Master: Indicating to P%d that data can be read from " \
		"address %d\n", simPid, address);
}

// Logs that a queued write request fulfillment was indicated to a process
void logWriteIndication(int simPid, int address){
	if (++lines > MAX_LOG_LINES) return;

	fprintf(log, "Master: Indicating to P%d that write has happened to " \
		"address %d\n", simPid, address);
}

// Logs that a queued read or wite reference was fulfilled
void logGrantedQueuedRequest(int simPid, Reference ref){
	if (ref.type == READ_REFERENCE)
		logReadIndication(simPid, ref.address);
	else
		logWriteIndication(simPid, ref.address);
}

/*
// Logs the detection of a resource request
void logRequestDetection(int simPid, int resourceId, int count, Clock time){
#ifdef VERBOSE
	if (++lines > MAX_LOG_LINES) return;


	fprintf(log, "Master has detected Process P%d requesting %d of R%d at" \
		" time %03d : %09d\n", simPid, count, resourceId, 
		time.seconds, time.nanoseconds);
#endif
}

// Logs allocation of a resource
void logAllocation(int simPid, int resourceId, int count, Clock time){
	statsRequestGranted(); // Records that a resource request was granted

#ifdef VERBOSE
	if (++lines > MAX_LOG_LINES) return;

	fprintf(log, "Master granted P%d request for %d of R%d at time " \
		" %03d : %09d\n", simPid, count, resourceId, time.seconds, 
		time.nanoseconds);

#endif
}

// Logs when a request is denied and placed in a queue for a resource
void logEnqueue(int simPid, int quantity, int rNum, int available){
#ifdef VERBOSE
	if (++lines > MAX_LOG_LINES) return;

	fprintf(log, "\tP%d requested %d of R%d but only %d available, " \
		"enqueueing request\n", simPid, quantity, rNum, available);
#endif
}

// Prints the resource allocation table every 20 requests by default
void logTable(const ResourceDescriptor * resources){
#ifdef VERBOSE
	if (lines >= MAX_LOG_LINES) return;
	static int callCount = 0;	// Times called since last print

	// Restricts printing to once every 20 request by default
	callCount++;
	if (callCount < ALLOC_PER_TABLE) return;
	callCount = 0;

	// Prints the table	
	int m, n;	// m resources, n processes
	
	// Prints header
	fprintf(log, "\n     ");
	lines++;

	for (m = 0; m < NUM_RESOURCES; m++){
		fprintf(log, "R%02d ", m);
	}

	fprintf(log, "\n");
	lines++;

	// Prints rows
	for (n = 0; n < MAX_RUNNING; n++){
		fprintf(log, "P%02d: ", n);

		// Prints each resource
		for (m = 0; m < NUM_RESOURCES; m++)
			fprintf(log, "%02d  ", resources[m].allocations[n]);

		fprintf(log, "\n");
		lines++;
	}

	fprintf(log, "\n");
	lines++;
#endif
}

// Logs the id and quantity of resources being released at a particular time
void logResourceRelease(int simPid, int resourceId, int count, Clock time){
#ifdef VERBOSE
	if (++lines > MAX_LOG_LINES) return;

	fprintf(log, "Master has acknowledged Process P%d releasing %d of R%d" \
		" at time %03d : %09d\n", simPid, count, resourceId, 
		time.seconds, time.nanoseconds);
#endif
}

// Prints a line that deadlock detection is being run
void logDeadlockDetection(Clock time){
	statsDeadlockDetectionRun(); // Tallies times deadlock detection run

	if (++lines > MAX_LOG_LINES) return;

	fprintf(log, "Master running deadlock detection at time %03d : %09d:" \
		"\n", time.seconds, time.nanoseconds);

}

// Prints the pids of processes in deadlock
void logDeadlockedProcesses(int * deadlockedPids, int size){
	if (++lines > MAX_LOG_LINES || size < 1) return;

	fprintf(log, "\tProcesses P%d", deadlockedPids[0]);

	int i = 1;
	for ( ; i < size; i++){
		fprintf(log, ", P%d", deadlockedPids[i]);
	}

	fprintf(log, " deadlocked\n");
}

// Prints that a deadlock resolution attempt is being made
void logResolutionAttempt(){
	// Prints resolution to log file if 
	if (++lines > MAX_LOG_LINES) return;
	fprintf(log, "\tAttempting to resolve deadlock...\n");
}

// Prints a message indicating that a process with logical pid was killed
void logKill(int simPid){
	statsProcessKilled();	// Records that a process was killed

	if (++lines > MAX_LOG_LINES) return;

	fprintf(log, "\tKilling process P%d\n", simPid);
}

// Prints a message indicating that deadlock has been resolved
void logResolutionSuccess(int killed, int runningAtStart){
	statsDeadlockResolved(killed, runningAtStart);	

	if (++lines > MAX_LOG_LINES) return;

	fprintf(log, "\tSystem is no longer deadlocked.\n");
}

// Prints a message indicating that a process has terminated on its own
void logCompletion(int simPid){
	statsProcessCompleted(); // Records that process terminated successfully

#ifdef VERBOSE
	if (++lines > MAX_LOG_LINES) return;

	fprintf(log, "\tMaster has responded to P%d completing\n", simPid);
#endif
}

// Prints the resource class ids and quantity of released resources
void logRelease(int * resources){
	int released[NUM_RESOURCES];	// Number of each resource reached
	int indices[NUM_RESOURCES];	// Index of each resource

	// Copies non-zero allocations & indices to released, returns if zero
	int i, j = 0;
	for (i = 0; i < NUM_RESOURCES; i++){
		if (resources[i] != 0){
			released[j] = resources[i];
			indices[j] = i;
			j++;
		}
	}

	// Returns if no resources released
	if (j == 0) return;

	// Returns if max log lines reached	
	if (++lines > MAX_LOG_LINES) return;

	// Prints message, first resource released and quantity
	fprintf(log, "\t\tResources released are as follows: R%d:%d",
		indices[0], released[0]);

	// Prints subsequent resources released and quantity
	for (i = 1; i < j; i++){
		fprintf(log, ", R%d:%d", indices[i], released[i]);
	}
	fprintf(log, "\n");
}

// Prints table of m resources, n processes
int printTable(FILE * fp, const int * table, int m, int n){
	// Prints the table	
	int r, p;	// resource and process indices
	int lines = 0;	// Number of lines added
	
	// Prints header of resources
	fprintf(fp, "\t     ");
	for (r = 0; r < m; r++){
		fprintf(fp, "R%02d ", r);
	}
	fprintf(fp, "\n");
	lines++;

	// Prints rows of table
	for (p = 0; p < n; p++){
		fprintf(fp, "\tP%02d: ", p);

		// Prints each resource
		for (r = 0; r < m; r++)
			fprintf(fp, "%02d  ", table[p*m + r]);

		fprintf(fp, "\n");
		lines++;
	}
	fprintf(fp, "\n");
	lines++;

	return lines;
}

// Prints allocated, requested, and available matrices to a file, returns num \n
int printMatrices(FILE * fp, const int * allocated, const int * request, 
		  const int * available){

	// Rare instance where not having a named constant is probably best
	int addedLines = 6;
	
        fprintf(fp, "\n\tAllocation matrix:\n");
        addedLines += printTable(fp, allocated, NUM_RESOURCES, MAX_RUNNING);

        fprintf(fp, "\n\tRequest matrix:\n");
        addedLines += printTable(fp, request, NUM_RESOURCES, MAX_RUNNING);

        fprintf(fp, "\n\tAvailable vector:\n");
	addedLines += printTable(fp, available, NUM_RESOURCES, 1);

	return addedLines;
}

// Prints a matrix representation of the state of the program to a file
int printMatrixRep(FILE * fp, const ResourceDescriptor * resources){
        int allocated[NUM_RESOURCES * MAX_RUNNING];     // Resource allocation
        int request[NUM_RESOURCES * MAX_RUNNING];       // Current requests
        int available[NUM_RESOURCES];                   // Available resources

        // Initializes vectors
        setAllocated(resources, allocated);
        setRequest(resources, request);
        setAvailable(resources, available);

	// Prints matrices
	return printMatrices(fp, allocated, request, available);
}

// Logs a matrix representation of the system state
void logMatrixRep(const ResourceDescriptor * resources){
	if (lines > MAX_LOG_LINES) return;

	lines += printMatrixRep(log, resources);
}

// Logs allocated, requested, and available matrices
void logMatrices(const int * allocated, const int * request, 
		 const int * available){
	if (lines > MAX_LOG_LINES) return;

	lines += printMatrices(log, allocated, request, available);

}

// Prints statistics to the log file at the end of a run
void logStats(){
	Stats stats = getStats();

	fprintf(log, "\nSTATS:\n" \
		"Total requests granted: %lu\n" \
		"Processes terminated by deadlock detection/recovery: %lu\n" \
		"Processes terminated successfully: %lu\n" \
		"Times deadlock detection run: %lu\n\n" \
		"%f percent of processes terminated per deadlock on average.",
		stats.numRequestsGranted,
		stats.numProcessesKilled,
		stats.numProcessesCompleted,
		stats.numTimesDeadlockDetectionRun,
		stats.percentKilledPerDeadlock);
}*/
