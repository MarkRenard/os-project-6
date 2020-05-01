// logging.h was created by Mark Renard on 4/13/2020.
//
// This file contains headers for functions that aid in the collection,
// formatting, and logging of data pertinent to Assignment 5.

#ifndef LOGGING_H
#define LOGGING_H

// Opens the log file with name LOG_FILE_NAME or exits with an error message
void openLogFile();

// Closes the log file
void closeLogFile();

// Logs when a process has terminated
void logTermination(int simPid, Clock time);

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
void logSwapping(int frameNum, int simPid, int pageNum);

/*
// Logs the detection of a resource request
void logRequestDetection(int simPid, int resourceId, int count, Clock time);

// Logs allocation of a resource
void logAllocation(int simPid, int resourceId, int count, Clock time);

// Logs when a request is denied and placed in a queue for a resource
void logEnqueue(int simPid, int quantity, int rNum, int available);

// Prints the resource allocation table every 20 requests by default
void logTable(ResourceDescriptor * resources);

// Logs the ids and quantities of resources being released at a particular time
void logResourceRelease(int simPid, int resourceId, int count, Clock time);

// Prints a line that deadlock detection is being run
void logDeadlockDetection(Clock time);

// Prints the pids of processes in deadlock
void logDeadlockedProcesses(int * deadlockedPids, int size);

// Prints that a deadlock resolution attempt is being made
void logResolutionAttempt();

// Prints a message indicating that deadlock has been resolved
void logResolutionSuccess();

// Prints a message indicating that a process has terminated on its own
void logCompletion(int simPid);

// Prints a message indicating that a process with logical pid was killed
void logKill(int simPid);

// Prints the resource class ids and count of released resources
void logRelease(int * resources);

// Prints table of m resources, n processes
int printTable(FILE * fp, int * table, int m, int n);

// Prints allocated, requested, and available matrices to a file
int printMatrices(FILE * fp, const int * allocated, const int * requested,
		  const int * available); 

// Prints a matrix representation of the state of the program to a file
int printMatrixRep(FILE * fp, const ResourceDescriptor * resources);

// Logs a matrix representation of the system state
void logMatrixRep(const ResourceDescriptor * resources);

// Logs allocated, requested, and available matrices
void logMatrices(const int * allocated, const int * requested,
                 const int * available);

// Prints statistics to the log file at the end of a run
void logStats();
*/

#endif
