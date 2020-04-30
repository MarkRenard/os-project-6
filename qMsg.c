// qMsg.c was created by Mark Renard on 3/28/2020 and subsequently renamed.
//
// This file contains implementations of utility functions which aid in the
// use of a message queue to send and recieve messages.

#include <errno.h>
#include <stdio.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <string.h>

#include "qMsg.h"
#include "perrorExit.h"

// Returns the message queue id of a new message queue
int getMessageQueue(int key, int flags){
	int msgQueueId;
	if ((msgQueueId = msgget(key, flags)) == -1)
        	perrorExit("Failed to create message queue");

	return msgQueueId;
}

// Adds a message to the message queue with the specified message queue id
void sendMessage(int msgQueueId, const char * msgText, long int type){
	qMsg msg;	// Buffer for the message to be sent

	// Initializes message
	msg.type = type;
	strcpy(msg.str, msgText);

	// Sends message
	if ((msgsnd(msgQueueId, (const void *)&msg, sizeof(msg.str), 0)) == -1){
		fprintf(stderr, "Couldn't send msg of type %ld\n", type);
		fprintf(stderr, "Msg: %s\n", msg.str);
		perrorExit("Couldn't send message");
	}
}

// Blocks until a message of the selected type is recieved in the selected queue
void waitForMessage(int msgQueueId, char * msgText, long int type){
	qMsg msg;	// Buffer for message to be received

	// Waits for message
	if ((msgrcv(msgQueueId, (void *)&msg, \
		sizeof(msg.str), type, 0)) == -1)
			perrorExit("Error waiting for message");

	// Copies message text
	strcpy(msgText, msg.str);
}

// Checks to see if a message has been sent, doesn't block if not
int getMessage(int msgQueueId, char * msgText, long int * type ){
	qMsg msg;	// Buffer for message to be recieved

	if(msgrcv(msgQueueId, (void *)&msg, sizeof(msg.str), 0, IPC_NOWAIT) \
		== -1){
		if (errno == ENOMSG) return 0;
		else perrorExit("Error getting message");
	}

	strcpy(msgText, msg.str);
	*type = msg.type;
	return 1;

}

// Removes the message queue with the specified id
void removeMessageQueue(int msgQueueId){
	if ((msgctl(msgQueueId, IPC_RMID, NULL)) == -1)
		perrorExit("Error removing message queue");
}
