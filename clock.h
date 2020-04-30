// clock.h was created by Mark Renard on 2/21/2020 and modified on 4/17/2020.
//
// This file defines a type used to simulate a clock in a shared memory region.

#ifndef CLOCK_H
#define CLOCK_H

#include <stdio.h>

// Uses two unsigned ints as a clock
typedef struct clock {
	unsigned int seconds;
	unsigned int nanoseconds;
} Clock;

Clock zeroClock();
Clock newClock(unsigned int seconds, unsigned int nanoseconds);
Clock randomTime(Clock min, Clock max);
void copyTime(Clock * dest, const Clock src);
void incrementClock(Clock * clock, const Clock increment);
int clockCompare(const Clock clk1, const Clock clk2);
Clock clockSum(Clock t1, Clock t2);
Clock clockDiff(Clock t1, Clock t2);
long double clockRatio(Clock t1, Clock t2);
void printTime(FILE * fp, const Clock clock);
void printTimeln(FILE * fp, const Clock clock);

#endif
