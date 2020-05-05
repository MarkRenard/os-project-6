// getOption.c was created by Mark Renard on 5/4/2020.
//
// This file defines a function wich returns 1 or 0, depending on what the user
// entered as an optarg for -m.

#include "perrorExit.h"
#include "constants.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


// Prints usage message on incorrect usage and exits
static void printUsageExit(){
	fprintf(stderr, "\nusage: \n\t%s -m 0 for unweighted "
		"addresss\n\t%s -m 1 for weighted address "
		"selection\n", exeName, exeName);
	exit(1);
}

// True if optarg is neither "0" nor "1"
static int invalidOptarg(char * optarg){
	return strcmp("0", optarg) != 0 && strcmp("1", optarg) != 0;
}

// Returns the optarg the user enters after argument -m or exits with usage msg
char * getOption(int argc, char * argv[]){
	int option;
	char * arg = NULL;

	// Retreives options, checking for invalid arguments
	while((option = getopt(argc, argv, "m:")) != -1){
		switch (option){
		case 'm':

			// Prints usage message and exits if optarg invalid
			if (invalidOptarg(optarg)) printUsageExit();

			// Copies optarg
			arg = optarg;	
			break;

		default:
			printUsageExit();
		}
	}

	// Prints usage message and exits if no valid optarg entered
	if (arg == NULL) printUsageExit();
	
	return arg;

}
