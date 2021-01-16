#ifndef socketutil_h
#define socketutil_h

#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <errno.h> 
#include <netdb.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 

void checkHostName(int hostname);

void checkHostEntry(struct hostent * hostentry);

void checkIPbuffer(char *IPbuffer);

#endif
