// stats.c was created by Mark Renard on 5/3/2020.
//
// This file defines functions that log statistics related to assignment 6.

#include "stats.h"
#include "clock.h"

static unsigned long int totalMemoryAccesses = 0;
static unsigned long int totalPageFaults = 0;
static Clock totalMemoryAccessTime = {0, 0};

Stats getStats(Clock currentTime){
	Stats stats;			// Statistics to be returned
	long double accessSeconds;	// Total memory access time in seconds
	long double totalSeconds;	// Total execution time in seconds

	accessSeconds = clockSeconds(totalMemoryAccessTime); 
	totalSeconds = clockSeconds(currentTime);

	// Computes memory accesses per second
	stats.memoryAccessesPerSecond = totalMemoryAccesses / totalSeconds;

	// Computes page faults per memory access
	stats.pageFaultsPerMemoryAccess = (double) totalPageFaults \
					  / (double) totalMemoryAccesses;

	// Computes average memory access speed
	stats.averageMemoryAccessSpeed = accessSeconds / totalMemoryAccesses;

	fprintf(stderr, "\n\ntotalMemoryAccesses: %lu\n" \
			"totalPageFaults: %lu\n" \
			"totalMemoryAccessTime: %03d : %09d\n\n" \

			"currentTime: %03d : %09d\n" \
			"accessSeconds: %Lf\n" \
			"totalSeconds: %Lf\n",
			totalMemoryAccesses,
			totalPageFaults,
			totalMemoryAccessTime.seconds,
			totalMemoryAccessTime.nanoseconds,
			currentTime.seconds,
			currentTime.nanoseconds,
			accessSeconds,
			totalSeconds);

	return stats;
}

void statsAddMemoryAccessTime(Clock time){
	incrementClock(&totalMemoryAccessTime, time);
}

void statsPageFault(){
	totalPageFaults++;
}

void statsMemoryAccess(){
	totalMemoryAccesses++;
}

/*
static long double percentageAcc = 0.0;

void initStats(){
        stats.numRequestsGranted = 0;
        stats.numProcessesKilled = 0;
        stats.numProcessesCompleted = 0;
        stats.numTimesDeadlockDetectionRun = 0;
        stats.numTimesDeadlocked = 0;

	stats.percentKilledPerDeadlock = -1.0;
}

// Records the number of times oss grants a request for resources
void statsRequestGranted(){
	stats.numRequestsGranted++;
}

// Records the number of times oss terminates a process
void statsProcessKilled(){
	stats.numProcessesKilled++;
}

// Records the number of times a process terminates successfully
void statsProcessCompleted(){
	stats.numProcessesCompleted++;
}

// Records the number of times deadlock detection is run
void statsDeadlockDetectionRun(){
	stats.numTimesDeadlockDetectionRun++;
}

// Records number of times deadlock detected and percentage of processes killed
void statsDeadlockResolved(int killed, int runningAtStart){
	percentageAcc += (double)killed/(double)runningAtStart;

	stats.numTimesDeadlocked++;
}

// Sets the percentage of running processes terminated per deadlock on average
static void setPercentKilled(Stats * st){
	st->percentKilledPerDeadlock = (double)percentageAcc \
		/ (double)st->numTimesDeadlocked * 100;
}

Stats getStats(){
	setPercentKilled(&stats);
	return stats;
}*/
