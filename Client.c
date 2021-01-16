#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>
#include "socketutil.h"

static int connected = 0;

pthread_t wtid;
void * waitmessage(void * arg) {
	while(1) {
		sleep(2);
		if (connected == 0) {
			printf("Attempting to connect...\n");
			sleep(8);
		}
		else {
			pthread_exit(0);
		}
	}
}

int main(int argc, char ** argv) {
	int	csd;
	struct addrinfo	addrinfo;
	struct addrinfo *result;
	char string[512];
	char buffer[512];
	char prompt[] = ">> ";
	char output[512];
	int	errcode;
	int	len;

	addrinfo.ai_flags = 0;			// active client does connecting
	addrinfo.ai_family = AF_INET;		// IPv4 only
	addrinfo.ai_socktype = SOCK_STREAM;	// Want TCP/IP
	addrinfo.ai_protocol = 0;		// Any protocol
	addrinfo.ai_addrlen = 0;
	addrinfo.ai_addr = NULL;
	addrinfo.ai_canonname = NULL;
	addrinfo.ai_next = NULL;
	if (argc < 2) {
		fprintf(stderr, "\x1b[1;31mNo host name specified.  File %s line %d.\x1b[0m\n", __FILE__, __LINE__);
		exit(1);
	}
	else if (argc < 3) {
		fprintf(stderr, "\x1b[1;31mNo port specified.  File %s line %d.\x1b[0m\n", __FILE__, __LINE__);
		exit(1);
	}
	else if ((errcode = getaddrinfo(argv[1], argv[2], &addrinfo, &result)) != 0) {
		fprintf(stderr, "\x1b[1;31mgetaddrinfo( %s, %s ) failed errno is %s  File %s line %d.\x1b[0m\n", argv[1],
			argv[2], gai_strerror(errcode), __FILE__, __LINE__);
		return -1;
	}
	else if (errno = 0, (csd = socket(result->ai_family, result->ai_socktype, result->ai_protocol)) == -1) {
		fprintf(stderr, "\x1b[1;31msocket() failed errno is %s  File %s line %d.\x1b[0m\n", strerror(errno), __FILE__, __LINE__);
		freeaddrinfo(result);
		return -1;
	}
	pthread_create(&wtid, NULL, waitmessage, NULL);
	if (errno = 0, connect(csd, result->ai_addr, result->ai_addrlen) == -1) {
		fprintf(stderr, "\x1b[1;31mFailed to establish connection (%s).\x1b[0m\n", strerror(errno));
		freeaddrinfo(result);
		return -1;
	}
	else {
		freeaddrinfo(result);
		printf("Connected to server %s\n", argv[1]);
		connected = 1;
		while (write(1, prompt, sizeof(prompt)), (len = read(0, string, sizeof(string))) > 0) {
			string[len-1]= '\0';
			write(csd, string, strlen(string) + 1);
			read(csd, buffer, sizeof(buffer));
			sprintf(output, "Server > %s\n", buffer);
			write(1, output, strlen(output));
		}
		close(csd);
		return 0;
	}
}
