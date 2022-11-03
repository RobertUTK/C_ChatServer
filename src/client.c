//This file contains the function definitions for a client.
//Author: Robert Schaffer
#include <stdlib.h>
#include <string.h>
#include "client.h"

//initClient: Allocates a new client
//Returns: c, a pointer to a client
extern client_t *
initClient()
{
	client_t *c;
	c = (client_t*) malloc(sizeof(client_t));
	return c;
}

//setClientName: Allocates and sets a clients name based on the string sent
//Params: c, a pointer to the client
//		  name, the name to be set
//Post-Con: The client's name string will be allocated and set
extern void
setClientName(client_t *c, char *name)
{	
	int len = strlen(name) + 1;
	c->name = (char*) malloc(sizeof(char)*len);
	memcpy(c->name, name, len);
}

//freeClient: Frees the memory allocated for a client;
//Param: c, a pointer to the client
//Post-Con: The data allocated for the client will be freed
extern void
freeClient(client_t * c)
{
	free(c->name);
	free(c);
}
