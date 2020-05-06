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
			"totalSeconds: %Lf\n\n" \

			"memoryAccessesPerSecond: %Lf\n" \
			"pageFaultsPerMemoryAccess: %Lf\n" \
			"averageMemoryAccessSpeed: %Lf\n\n",

			totalMemoryAccesses,
			totalPageFaults,
			totalMemoryAccessTime.seconds,
			totalMemoryAccessTime.nanoseconds,

			currentTime.seconds,
			currentTime.nanoseconds,
			accessSeconds,
			totalSeconds,

			stats.memoryAccessesPerSecond,
			stats.pageFaultsPerMemoryAccess,
			stats.averageMemoryAccessSpeed);

	return stats;
}

void statsAddMemoryAccessTime(Clock time){
	incrementClock(&totalMemoryAccessTime, time);
/*	fprintf(stderr, "totalMemoryAccessTime: %03d : %09d\n",
		totalMemoryAccessTime.seconds,
		totalMemoryAccessTime.nanoseconds);
*/
}

void statsPageFault(){
	totalPageFaults++;
}

void statsMemoryAccess(){
	totalMemoryAccesses++;
}

