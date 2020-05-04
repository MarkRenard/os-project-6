// stats.h was created by Mark Renard on 4/18/2020.
//
// This file contains the definition of a struct with collected statistics on
// the execution of oss in assignment 5, along with functions for collecting them.

#ifndef STATS_H
#define STATS_H

#include "clock.h"

typedef struct stats {
	long double memoryAccessesPerSecond;
	long double pageFaultsPerMemoryAccess;
	long double averageMemoryAccessSpeed;
} Stats;

Stats getStats(Clock currentTime);
void statsAddMemoryAccessTime(Clock time);
void statsPageFault();
void statsMemoryAccess();


#endif
