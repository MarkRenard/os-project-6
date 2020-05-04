// constants.h was created by Mark Renard on 4/30/2020
//
// This file contains definitions of constants used in assignment 6 of the
// spring 2020 semester of 4760, grouped according to the source files in which
// they are used.

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <sys/msg.h>
#include <sys/stat.h>


// Miscelaneous
#define MAX_RUNNING 18 	 		// Max number of running child processes
#define MAX_LAUNCHED 100		// Max total children launched

#define PAGE_SIZE 1024			// Size of one page in bytes
#define NUM_FRAMES 256			// Total frames in main memory
#define MIN_ALLOC_PAGES 1		// Min number of pages per process
#define MAX_ALLOC_PAGES 32		// Max number of pages per process

#define BILLION 1000000000U		// The number of nanoseconds in a second
#define MILLION 1000000U		// Number of nanoseconds per millisecond
#define BUFF_SZ 100			// The size of character buffers 
#define MSG_SZ 30			// Size of qMsg char arrays

#define EMPTY -1			// Generic sentinel for unset values


// Used by oss.c
#define MIN_FORK_TIME_SEC 0U		// Value of seconds in MIN_FORK_TIME
#define MIN_FORK_TIME_NS (1 * MILLION)	// Value of nanoseconds in MIN_FORK_TIME
#define MAX_FORK_TIME_SEC 0U		// Value of seconds in MAX_FORK_TIME
#define MAX_FORK_TIME_NS (500 * MILLION)// Value of nanoseconds in MAX_FORK_TIME

#define SLEEP_NS 500000U		// Real sleep between simulation loops

#define USER_PROG_PATH "./userProgram"	// The path to the user program

#define LOOP_INCREMENT_SEC 0		// System clock increment per loop sec
#define LOOP_INCREMENT_NS (50 * MILLION)// System clock increment per loop ns

#define MEM_ACCESS_SEC 0		// Time to access main memory seconds
#define MEM_ACCESS_NS 10		// Time to access main memory nanosec

#define IO_OPERATION_SEC 0		// Seconds to perform disk read/write
#define IO_OPERATION_NS (14 * MILLION)	// Disk read/write nanoseconds

#define MEM_MAP_PRINT_INTERVAL_SEC 1	// Interval between memory map prints sec
#define MEM_MAP_PRINT_INTERVAL_NS 0	// Interval between memory map prints ns


// Used by userProgram.c
#define READ_PROBABILITY 0.8		// Chance of read instead of write

#define MIN_REFERENCES 90		// Min references before terminating
#define MAX_REFERENCES 110		// Max before chance of termination

#define MIN_REF_INTERVAL_SEC 0		// Min time between references sec
#define MIN_REF_INTERVAL_NS 1		// Min time between references nanosec
#define MAX_REF_INTERVAL_SEC 0		// Max time between references sec
#define MAX_REF_INTERVAL_NS 10		// Max time between references nanosec

#define TERMINATION_PROBABILITY 0.99	// Chance to terminate after references

#define CLOCK_UPDATE_SEC 0		// System clock increment for user sec
#define CLOCK_UPDATE_NS 10		// System clock increment for user ns


// Used by both oss.c and userProgram.c
#define REQUEST_MQ_KEY 59597192		// Message queue key for requests
#define REPLY_MQ_KEY 38257848		// Message queue key for replies
#define MQ_PERMS (S_IRUSR | S_IWUSR)	// Message queue permissions

#define BASE_SEED 39393984		// Used in calls to srand

#define TERMINATE (MAX_ALLOC_PAGES * PAGE_SIZE + 1)  // Termination sentinel
#define NO_MESSAGE (MAX_ALLOC_PAGES * PAGE_SIZE + 2) // No message sentinel


// Used by bitVector.c to track allocated frames
#define NUM_BITS (sizeof(unsigned int) * 8)  // Bits per unsigned int
#define MAX_VALUE (NUM_FRAMES - 1) 	   // Max int tracked in bit vector
#define BIT_VECTOR_SIZE (MAX_VALUE / NUM_BITS + 1) // Size of bit vector


// Used by logging.c
#define LOG_FILE_NAME "oss_log"		// The name of the log file
#define MAX_LOG_LINES 1000000		// Max number of lines in the log file

#endif
