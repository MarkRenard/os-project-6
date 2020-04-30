// perrorExit.c was created by Mark Renard on 2/21/2020

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

char * exeName;

// This function prints an error message in a standard format and exits.
void perrorExit(char * msg){
	char errmsg[100];
	sprintf(errmsg, "%s: Error: %s", exeName, msg);
	perror(errmsg);

	kill(0, SIGINT);
}
