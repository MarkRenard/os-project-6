// qMsg.h was created by Mark Renard on 3/27/2020 and modified on 4/15/2020.
//
// This file contains the definition of a struct used to pass messages between
// oss and user processes and headers for message queue utility functions.

#ifndef QMSG_H
#define QMSG_H

#include "constants.h"

typedef struct qmsg {
	long int type;
	char str[MSG_SZ];
} qMsg;

int getMessageQueue(int key, int flags);
void sendMessage(int msgQueueId, const char * msgText, long int type);
void waitForMessage(int msgQueueId, char * msgText, long int type);
int getMessage(int msgQueueId, char * msgText, long int * type);
void removeMessageQueue(int msgQueueId);

#endif

