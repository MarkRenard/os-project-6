// logging.h was created by Mark Renard on 4/13/2020.
//
// This file contains headers for functions that aid in the collection,
// formatting, and logging of data pertinent to Assignment 5.

#ifndef LOGGING_H
#define LOGGING_H

#include "pcb.h"
#include "frameDescriptor.h"
#include "clock.h"

// Opens the log file with name LOG_FILE_NAME or exits with an error message
void openLogFile();

// Closes the log file
void closeLogFile();

// Logs when a process has terminated
void logTermination(int simPid, Clock time, const PCB * pcb);

// Logs when a request was received by oss
void logRequest(int simPid, Reference ref, Clock time);

// Logs when a request is immediately granted by oss
void logGrantedRequest(Reference ref, unsigned char frameNum, int simPid,
		       Clock time);

// Logs a request to read from an address at a particular time
void logReadRequest(int simPid, int address, Clock time);

// Logs the granting of a request to read from an address
void logReadGranted(int address, int frameNum, int simPid, Clock time);

// Logs a request to write to an address at a particular time
void logWriteRequest(int simPid, int address, Clock time);

// Logs the granting of a request to write to an address
void logWriteGranted(int address, int frameNum, int simPid, Clock time);

// Logs a page fault event
void logPageFault(int address);

// Logs swapping in of a page
void logSwap(int frameNum, int simPid, int pageNum);

// Logs that a frame was dirty
void logDirty(int frameNum);

// Logs that a queued read request fulfillment was indicated to a process
void logReadIndication(int simPid, int address);

// Logs that a queued write request fulfillment was indicated to a process
void logWriteIndication(int simPid, int address);

// Logs that a queued read or wite reference was fulfilled
void logGrantedQueuedRequest(int simPid, Reference ref);

// Prints a representation of the page table of each process to teh log
void logPages(const PCB * pcbs);

// Prints a representation of frame data to the log
void logFrames(const FrameDescriptor * frameTable);

// Prints the memory map of the system to the log
void logMemoryMap(const PCB * pcbs, FrameDescriptor * frameTable, Clock time);

// Logs memory access statistics
void logStats(Clock time);

#endif
