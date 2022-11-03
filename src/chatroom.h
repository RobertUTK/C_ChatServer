//This file contains the structure and function prototypes for a chatroom
//Author: Robert Schaffer
#ifndef CHATROOM_H
#define CHATROOM_H

#include "jrb.h"
#include "dllist.h"
#include <pthread.h>

typedef struct {
	char *name;
	Dllist clients;
	Dllist out;
	pthread_t tipd;
	pthread_cond_t cv;
	pthread_mutex_t lock;
} chatroom_t;

extern chatroom_t *initRoom(char *);
extern void freeRoom(chatroom_t *);

#endif
