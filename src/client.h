/*This file contains the structure and function prototypes for a client.
 *Author: Robert Schaffer
 */
#ifndef CLIENT_H
#define CLIENT_H

#include <pthread.h>
#include <stdio.h>
#include "chatroom.h"
#include "dllist.h"

typedef struct {
	char *name;
	int *fdp;
	FILE* fout;
	FILE* fin;
	chatroom_t *room;
} client_t;

extern client_t *initClient();
extern void setClientName(client_t *, char *);
extern void freeClient(client_t *);

#endif
