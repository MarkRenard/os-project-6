// protectedClock.h was created by Mark Renard on 4/11/2020.
//
// This file defines a type used to extend the base clock type to allow
// mutually exclusive access by multiple processes for writing.

#ifndef PROTECTEDCLOCK_H
#define PROTECTEDCLOCK_H

#include <stdio.h>
#include <pthread.h>
#include "clock.h"

typedef struct protectedClock {
	Clock time;
	pthread_mutex_t sem;
} ProtectedClock;

void initPClock(ProtectedClock * pClockPtr);
void incrementPClock(ProtectedClock * pClockPtr, Clock increment);
Clock getPTime(ProtectedClock * pClockPtr);

#endif
