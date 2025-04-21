CC=gcc
CFLAGS=-g -O1 -Wall
LDLIBS=-lpthread

all: client server

client: client.c csapp.c
server: server.c csapp.c

clean:
	rm -f *.o *~ *.exe client server csapp.o

