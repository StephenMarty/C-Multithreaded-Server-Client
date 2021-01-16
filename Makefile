CC = gcc
CFLAGS = -g -Wall
COMPILE = $(CC) $(CFLAGS)

all: Client Server

Client: Client.c socketutil.o
	$(COMPILE) -o Client Client.c socketutil.o -lpthread

Server: Server.c socketutil.o
	$(COMPILE) -o Server Server.c socketutil.o -lpthread

socketutil.o: socketutil.c socketutil.h
	$(COMPILE) -c socketutil.c

clean:
	rm -rf *.o Client Server
