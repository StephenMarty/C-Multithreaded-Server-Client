#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <semaphore.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "socketutil.h"

#define PORT "3000"
#define MAX_CONNECTIONS 10

pthread_t tid[MAX_CONNECTIONS];

int holdcsd[MAX_CONNECTIONS];
int connections = 0;

sem_t sigsem;

typedef struct tinput {
	int csd;
	int csdspot;
} tinput;

void help(char * astr) {
	int len = strlen(astr);
	if (len == 4) {
		printf("This prompt allows various commands to be entered to control the server.\n");
		printf("help command usage:\n<help commands> Show a list of all commands.\n");
		printf("<help <command name>> Show the usage of a specific command.\n");
		return;
	}
	if (len > 5 && astr[4] == ' ') {
		char *ret;
		ret = strchr(astr, ' ') + 1; //pointer arithmetic; remove space
		if (strcmp(ret, "commands") == 0) {
			printf("kill\nexit\nconnection count\n");
		}
		else if (strcmp(ret, "kill") == 0) {
			printf("kill: End the server process.\n");
		}
		else if (strcmp(ret, "exit") == 0) {
			printf("exit: Escape from the input prompt.\n");
		}
		else if (strcmp(ret, "connection count") == 0) {
			printf("connection count: Get the number of active connections.\n");
		}
		else {
			printf("Invalid command.\n");
		}
		return;
	}
}

void sigint_handler(void) {
	printf("\b\b"); //remove ^C
	int semval;
	sem_getvalue(&sigsem, &semval);
	if (semval == 0) {
		sem_post(&sigsem);
	}
}

void init_csd() {
	int i;
	for (i=0; i<MAX_CONNECTIONS; i++) {
		holdcsd[i] = -1;
	}
	return;
}

int findemptycsd() {
	int i;
	if (connections < MAX_CONNECTIONS) {
		for (i=0; i<MAX_CONNECTIONS; i++) {
			if (holdcsd[i] == -1) {
				return i;
			}
		}
	}
	return -1;
}

void * cmd_prompt(void * arg) {
	sem_wait(&sigsem);
	while (1) {
		char cmd[20];
		printf("  \nEnter command: ");
    	scanf(" %[^\n]", cmd);
		if (strncmp(cmd, "help", 4) == 0) {
			help(cmd);
		}
    	else if (strcmp(cmd, "kill") == 0) {
    		printf("Server terminating.\n");
    		exit(0);
    	}
    	else if (strcmp(cmd, "exit") == 0) {
    		printf("Exiting prompt.\n");
    		//break;
			sem_wait(&sigsem);
    	}
    	else if (strcmp(cmd, "connection count") == 0) {
    		printf("Connections: %d\n", connections);
    	}
    	else {
    		printf("Invalid command.\n");
    	}
    }
}

void * client_handler(void * arg) {
	tinput *getargs = arg;
	char message[256];
	char string[200];
	int limit, size;
	char temp;
	int i;
	connections++;
	while (read(getargs->csd, string, sizeof(string)) > 0) {
		write(1, message, sprintf(message, " CSD %d: %s\n", getargs->csd, string));
		size = strlen(string);
		limit = strlen(string)/2;
		for (i = 0; i<limit; i++) {
			temp = string[i];
			string[i] = string[size-i-1];
			string[size-i-1] = temp;
		}
		write(getargs->csd, string, strlen(string)+1);
	}
	holdcsd[getargs->csdspot] = -1;
	printf(" \x1b[0;33mCSD %d disconnected\x1b[0m\n", getargs->csd);
	close(getargs->csd);
	connections--;
	pthread_exit(0);
}

int main(int argc, char ** argv) {
	int	sd;
	socklen_t ic;
	int	csd;
	struct sockaddr_in senderAddr;
	struct addrinfo	request;
	struct addrinfo * result;
	int	on = 1;
	int	errcode;

	request.ai_flags = AI_PASSIVE; // for bind()
	request.ai_family = AF_INET; // IPv4 only
	request.ai_socktype = SOCK_STREAM; // Want TCP/IP
	request.ai_protocol = 0; // Any protocol
	request.ai_addrlen = 0;
	request.ai_addr = NULL;
	request.ai_canonname = NULL;
	request.ai_next = NULL;
	
	init_csd();
	
	if ((errcode = getaddrinfo( 0, PORT, &request, &result)) != 0) {
		fprintf(stderr, "\x1b[1;31mERROR: getaddrinfo() failed errno is %s  File %s line %d.\x1b[0m\n", gai_strerror(errcode), __FILE__, __LINE__);
		return -1;
	}
	else if (errno = 0, (sd = socket( result->ai_family, result->ai_socktype, result->ai_protocol)) == -1) {
		fprintf(stderr, "\x1b[1;31mERROR: socket() failed errno is %s  File %s line %d.\x1b[0m\n", strerror(errno), __FILE__, __LINE__);
		freeaddrinfo(result);
		return -1;
	}
	else if (errno = 0, setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1) {
		fprintf( stderr, "\x1b[1;31mERROR: setsockopt() failed errno is %s  File %s line %d.\x1b[0m\n", strerror(errno), __FILE__, __LINE__);
		freeaddrinfo(result);
		return -1;
	}
	else if (errno = 0, bind(sd, result->ai_addr, result->ai_addrlen) == -1) {
		fprintf(stderr, "\x1b[1;31mERROR: bind() failed errno is %s  File %s line %d.\x1b[0m\n", strerror( errno ), __FILE__, __LINE__);
		freeaddrinfo(result);
		close(sd);
		return -1;
	}
	else if (errno = 0, listen(sd, 10) == -1) {
		fprintf(stderr, "\x1b[1;31mERROR: listen() failed errno is %s  File %s line %d.\x1b[0m\n", strerror(errno), __FILE__, __LINE__);
		freeaddrinfo(result);
		close(sd);
		return -1;
	}
	else {
		char hostbuffer[256];
		char *IPbuffer;
		struct hostent *host_entry;
		int hostname; 

		// To retrieve hostname 
		hostname = gethostname(hostbuffer, sizeof(hostbuffer)); 
		checkHostName(hostname); 

		// To retrieve host information 
		host_entry = gethostbyname(hostbuffer); 
		checkHostEntry(host_entry); 

		// To convert an Internet network 
		// address into ASCII string 
		IPbuffer = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0]));
    	if (signal(SIGINT, (void (*)(int)) sigint_handler) == SIG_ERR) {
        	printf("ERROR: Unable to catch SIGINT\n");
        	exit(1);
    	}
		
		if (sem_init(&sigsem, 0, 0) != 0) {
			printf("ERROR: Failed to initialize semaphore.\n");
		}
		pthread_t cmdtid;
		pthread_create(&cmdtid, NULL, cmd_prompt, NULL);
		
		printf("Hostname: %s\n", hostbuffer);
		printf("Host IP: %s\n", IPbuffer);
		//Public IP stuff
		printf("Public IP: ");
		fflush(stdout); //need to flush stdout so echoed ip is on the same line
		if (system("wget -qO- https://ipecho.net/plain ; echo") == -1) {
			printf("Failed to get public IP.");
		}
		//End Public IP stuff
		printf("Server waiting for connections.\n");
		int currcon = 0;
		ic = sizeof(senderAddr);
		while ((csd = accept(sd, (struct sockaddr *)&senderAddr, &ic)) != -1) {
			tinput *nclient = malloc(sizeof(tinput));
			nclient->csd = csd;
			printf("\x1b[0;32mClient connected (CSD %d)\x1b[0m\n", csd);
			int csdspot = findemptycsd();
			nclient->csdspot = csdspot;
			if (csdspot != -1) {
				holdcsd[csdspot] = csd;
				pthread_create(&tid[currcon], NULL, client_handler, nclient);
				currcon++;
			}
		}
		close(sd);
		return 0;
	}
}
