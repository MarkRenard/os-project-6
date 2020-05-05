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

// Returns the optarg the user enters after argument -m or exits with usage msg
int getOption(int argc, char * argv[]){
	int option;
	int arg = -1;

	// Retreives options, checking for invalid arguments
	while((option = getopt(argc, argv, "m:")) != -1){
		switch (option){
		case 'm':
			if (optarg[0] != '0' && optarg[0] != '1')
				printUsageExit();
			arg = atoi(optarg);
			break;
		default:
			printUsageExit();
		}
	}

	// Exits with usage message if a valid optino argument wasn't entered
	if (!(arg == 0 || arg == 1)){
		printUsageExit();
	}

	return arg;
}
