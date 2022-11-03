//This file contains the main and other functions for a chat server.
//Author: Robert Schaffer
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include "chatroom.h"
#include "client.h"
#include "sockettome.h"
#include "jrb.h"

static void *cRoomsProcess(void *);
static void *clientProcess(void *);
static void removeNewline(char *);
static char *makeStr(char *);
static void removeClient(client_t *);

JRB cRooms;

int 
main(int argc, char **argv){
	
	JRB tmp;
	chatroom_t *c;
	int i, port , sock, fd, *fdp;
	pthread_t *tidp;
	Jval v;

	if(argc < 3){
		fprintf(stderr, "usage: %s port Chat-Room-Names ...\n", argv[0]);
		exit(1);
	}

	port = (int) strtol(argv[1], NULL, 10);
	if(!port || port < 50000){
		fprintf(stderr, "port must be >= 50000\n");
		exit(1);
	}

	cRooms = make_jrb();

	for(i = 2; i < argc; i++){
		c = initRoom(argv[i]);
		v.v = (void*) c;
		jrb_insert_str(cRooms, c->name, v);
		pthread_create(&c->tipd, NULL, cRoomsProcess, (void*) c);
	}

	sock = serve_socket(port);

	for(;;){
		fd = accept_connection(sock);
		fdp = (int*) malloc(sizeof(int));
		*fdp = fd;
		tidp = (pthread_t*) malloc(sizeof(pthread_t));
		pthread_create(tidp, NULL, clientProcess, fdp);
	}

	jrb_traverse(tmp, cRooms){
		c = (chatroom_t*) tmp->val.v;
		freeRoom(c);
	}
	
	jrb_free_tree(cRooms);

	return 0;
}

//clientProcess: The thread process for a client that gets clients name and desired room and
//collects the users input until they quit
//Param: v, a pointer to the connection file descriptor
//Returns: NULL
static void *
clientProcess(void *v)
{
	chatroom_t *room;
	client_t *c, *tmpC;
	char userIn[500], buf[500], *str;
	Dllist clientTmp;
	JRB tmp;

	pthread_detach(pthread_self());
	
	c = initClient();

	c->fdp = (int*) v;

	c->fin = fdopen(*c->fdp, "r");
	c->fout = fdopen(*c->fdp, "w");
	

	fputs("Chat Rooms:\n\n", c->fout);
	jrb_traverse(tmp, cRooms){
		room = (chatroom_t*) tmp->val.v;
		fputs(room->name, c->fout);
		fputs(":", c->fout);
		dll_traverse(clientTmp, room->clients){
			fputs(" ", c->fout);
			tmpC = (client_t*) clientTmp->val.v;
			fputs(tmpC->name, c->fout);
		}
		fputs("\n", c->fout);
	}
	fputs("\n", c->fout);

	room = NULL;
	do{
		if(fputs("Enter your chat name (no spaces):\n", c->fout) == EOF){ removeClient(c); return NULL;}
		if(fflush(c->fout) == EOF){ removeClient(c); return NULL;}
		fgets(userIn, 500, c->fin);
		if(feof(c->fin)){ removeClient(c); return NULL;}
		removeNewline(userIn);
	}
	while(memchr(userIn, ' ', strlen(userIn)) != NULL);
	setClientName(c, userIn);
	
	tmp = NULL;
	while(tmp == NULL) {
		if(fputs("Enter chat room:\n", c->fout) == EOF){ removeClient(c); return NULL;}
		if(fflush(c->fout) == EOF){ removeClient(c); return NULL;}
		fgets(userIn, 500, c->fin);
		if(feof(c->fin)){ removeClient(c); return NULL;}
		removeNewline(userIn);
		
		tmp = jrb_find_str(cRooms, userIn);
		if(tmp == NULL){ 
			if(fputs("No chat room ", c->fout) == EOF){ removeClient(c); return NULL;} 
			if(fputs(userIn, c->fout) == EOF){ removeClient(c); return NULL;}
			if(fputs("\n", c->fout) == EOF){ removeClient(c); return NULL;}
		}
		if(fflush(c->fout) == EOF){ removeClient(c); return NULL;}
	}
	c->room = (chatroom_t*) tmp->val.v;
	str = makeStr(c->name);
	dll_append(c->room->clients, new_jval_v(c));

	strcpy(buf, c->name);
	strcat(buf, " has joined\n");
	str = makeStr(buf);
	pthread_mutex_lock(&c->room->lock);
	dll_append(c->room->out, new_jval_s(str));
	pthread_cond_signal(&c->room->cv);
	pthread_mutex_unlock(&c->room->lock);

	
	fgets(userIn, 500, c->fin);
	while(!feof(c->fin)){
		strcpy(buf, c->name);
		strcat(buf, ": ");	
		strcat(buf, userIn);
		str = makeStr(buf);

		pthread_mutex_lock(&c->room->lock);
		dll_append(c->room->out, new_jval_s(str));
		pthread_cond_signal(&c->room->cv);
		pthread_mutex_unlock(&c->room->lock);
		
		fgets(userIn, 500, c->fin);
	}

	removeClient(c);
	return NULL;
}

//cRoomsProcess: The thread process for each room that waits for a signal from a client and 
//prints that output to all clients in room
//Param: v, a pointer to the chatroomm structure
//Returns: NULL
static void *
cRoomsProcess(void *v)
{
	chatroom_t *room;
	client_t *client;
	Dllist tmp;
	char *s;
	
	pthread_detach(pthread_self());

	room = (chatroom_t*) v;
	
	pthread_mutex_lock(&room->lock);
	while(1){
		while(!dll_empty(room->out)){
			s = room->out->flink->val.s;
			printf("String: %s", s);
			fflush(stdout);
			dll_traverse(tmp, room->clients){
				client = (client_t*) tmp->val.v;
				printf("Client: %s\n", client->name);
				fflush(stdout);
				fputs(s, client->fout);
				fflush(client->fout);
			}
			dll_delete_node(room->out->flink);
			free(s);
		}
		pthread_cond_wait(&room->cv, &room->lock);
	}

	pthread_mutex_unlock(&room->lock);
	return NULL;
}

//removeNewline: Removes a newline from a string
//Param: s, the string with the newline
//Post-Con: The first newline is replaced with the null character
static void
removeNewline(char *s)
{
	int i;
	for(i = 0; i < strlen(s); i++){
		if(s[i] == '\n'){
			s[i] = '\0';
			break;
		}
	}
}

//makeStr: Allocates a new string based on the string sent
//Param: s, a string
//Post-Con: a new string will be allocated and set with the data from s
//Returns: newStr, the newly allocated string
static char *
makeStr(char *s)
{
	char *newStr;
	int len;

	len = strlen(s) + 1;
	newStr = (char*) malloc(sizeof(char)*len);
	memcpy(newStr, s, len);
	return newStr;
}

//removeClient: Removes a client from a chatroom, sends the client has left message, and 
//frees client's allocated memory
//Param: client, the client to remove
//Post-Con: The client will be removed from the chat room and its mmemory wil be freed
static void
removeClient(client_t *client)
{
	char buf[500];
	client_t * tmpC;
	Dllist tmp;
	char *str;


	if(client->room != NULL){	
		dll_traverse(tmp, client->room->clients){
			tmpC = (client_t*) tmp->val.v;
			if(*tmpC->fdp == *client->fdp){
				dll_delete_node(tmp);
				printf("Deleted: %s\n", tmpC->name);
				fflush(stdout);
				break;
			}
		}
	
		strcpy(buf, client->name);	
		strcat(buf, " has left\n");
		str = makeStr(buf);
		printf("removeClient: %s", str);
		pthread_mutex_lock(&client->room->lock);
		printf("removeClient(inside mutex): %s", str);
		dll_append(client->room->out, new_jval_s(str));
		printf("removeClient(after append): %s", str);
		pthread_cond_signal(&client->room->cv);
		pthread_mutex_unlock(&client->room->lock);
	}
	fclose(client->fin);
	fclose(client->fout);
	close(*client->fdp);
	free(client->fdp);
	freeClient(client);
}
