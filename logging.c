// logging.c was created by Mark Renard on 5/1/2020.
//
// This file contains definitions for functions that aid in the collection,
// formatting, and logging of data pertinent to Assignment 6.

#include "clock.h"
#include "constants.h"
#include "pcb.h"
#include "perrorExit.h"
#include "stats.h"
#include <stdio.h>

static FILE * log = NULL;
static int lines = 0;

Clock MEM_ACCESS_TIME = {MEM_ACCESS_SEC, MEM_ACCESS_NS};

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

	// Tracks number of memory accesses
	statsMemoryAccess();

	if (ref.type == READ_REFERENCE)
		logReadRequest(simPid, ref.address, time);
	else
		logWriteRequest(simPid, ref.address, time);
}

// Logs when a request is immediately granted by oss
void logGrantedRequest(Reference ref, unsigned char frameNum, int simPid,
		       Clock time){

	// Tracks total memory access time
	Clock diff = clockDiff(time, ref.startTime); 
	statsAddMemoryAccessTime(diff);

	if (ref.type == READ_REFERENCE){
		logReadGranted(ref.address, frameNum, simPid, time);
	} else {
		logWriteGranted(ref.address, frameNum, simPid, time);
	}
}

// Logs a page fault event
void logPageFault(int address){
	if (++lines > MAX_LOG_LINES) return;

	// Tracks page faults
	statsPageFault();

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

	// Tracks memory access time
	Clock diff = clockDiff(ref.endTime, ref.startTime); 
	statsAddMemoryAccessTime(diff);

	if (ref.type == READ_REFERENCE)
		logReadIndication(simPid, ref.address);
	else
		logWriteIndication(simPid, ref.address);
}

// Prints the memory map of the system to the log
void logMemoryMap(const PCB * pcbs){
	if (lines + MAX_RUNNING + 2 > MAX_LOG_LINES) return;
	int i, j;

	// Prints frame numbers in header
	fprintf(log, "\n     ");
	for (j = 0; j < MAX_ALLOC_PAGES; j++)
		fprintf(log, "%2d ", j);
	fprintf(log, "\n");
	lines += 2;

	// Prints one row per process
	for (i = 0; i < MAX_RUNNING; i++){

		// Skips if the process is not running
		if (pcbs[i].realPid == EMPTY) continue;

		// Prints a row
		fprintf(log, "P%2d: ", i);

		for (j = 0; j < MAX_ALLOC_PAGES; j++){
			if (j < pcbs[i].lengthRegister){
				if (pcbs[i].pageTable[j].valid)
					fprintf(log, " + ");
				else
					fprintf(log, " . ");
			}
		}
		fprintf(log, "\n");
		lines++;
	}
	fprintf(log, "\n");
	lines++;	
	
}

// Logs memory access statistics
void logStats(Clock time){
	Stats stats = getStats(time);

	fprintf(log, "Number of memory accesses per second: %Lf\n" \
		"Number of page faults per memory access: %Lf\n" \
		"Average memory access speed: %Lf seconds per access",
		stats.memoryAccessesPerSecond,
		stats.pageFaultsPerMemoryAccess,
		stats.averageMemoryAccessSpeed);
}

