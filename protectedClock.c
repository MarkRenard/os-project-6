// protectedClock.c was created by Mark Renard at some point in the recent past.
//
// This file contains function implementations for accessing and mutating the
// values of a logical clock protected by a semaphore.

#include <pthread.h>

#include "clock.h"
#include "protectedClock.h"

// Initializes semaphore protecting the clock
static void initializeSemaphore(pthread_mutex_t * mutex){
        pthread_mutexattr_t attributes; // mutex attributes struct

        // Initializes mutex attributes struct
        pthread_mutexattr_init(&attributes);

        // Specifies that the semaphore can be used by multiple processes
        pthread_mutexattr_setpshared(&attributes, PTHREAD_PROCESS_SHARED);

        // Initializes the mutex with the attributes struct
        pthread_mutex_init(mutex, &attributes);
}

// Initializes a ProtectedClock
void initPClock(ProtectedClock * pClockPtr){
	pClockPtr->time = zeroClock();
	initializeSemaphore(&pClockPtr->sem);
}

// Locks and unlocks a semaphore to increment a ProtectedClock
void incrementPClock(ProtectedClock * pClockPtr, Clock increment){
	pthread_mutex_lock(&pClockPtr->sem);
	incrementClock(&pClockPtr->time, increment);
	pthread_mutex_unlock(&pClockPtr->sem);
}

// Returns the value of the time in a ProtectedClock
Clock getPTime(ProtectedClock * pClockPtr){
	Clock time;

	pthread_mutex_lock(&pClockPtr->sem);
	copyTime(&time, pClockPtr->time);
	pthread_mutex_unlock(&pClockPtr->sem);

	return time;
}


