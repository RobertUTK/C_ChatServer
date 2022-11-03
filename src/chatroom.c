//This file contains the function definitions for the chatroom functions
//Author: Robert Schaffer
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "chatroom.h"
#include "jrb.h"
#include "client.h"

//initRoom: Creates a new chatroom, allocates the needed data, 
//and initializes the condition variable and lock
//Param: roomName, a string with the room's name
//Post-Cons: The room and string for room name will be allocated
//			 The client and out lists will be created
//			 The lock and condition variable will be initialized
//Returns: c, the created chatroom
extern chatroom_t *
initRoom(char *roomName)
{
	chatroom_t *c;
	int len;

	c = (chatroom_t*) malloc(sizeof(chatroom_t));
	
	c->clients = new_dllist();
	c->out = new_dllist();
	
	len = strlen(roomName) + 1;
	c->name = (char*) malloc(sizeof(char)*len);
	memcpy(c->name, roomName, len);

	pthread_cond_init(&c->cv, NULL);
	pthread_mutex_init(&c->lock, NULL);
	return c;
}

//freeRoom: Frees the memory allocated for a room
//Param: c, the chatroom to free
//Post-Con: The memory for the chatroom will be freed
extern void
freeRoom(chatroom_t* c)
{
	Dllist tmp;
	client_t *client;

	free(c->name);
	dll_traverse(tmp, c->clients){
		client = (client_t*) tmp->val.v;
		freeClient(client);
	}
	free_dllist(c->out);

	free_dllist(c->clients);
	free(c);
}
