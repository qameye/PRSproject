CFLAGS = -g -Wall -DDEBUG
LDLAGS = -g -Wall -DDEBUG

all: main

main: server.o 
	gcc ${LDLAGS} server.o -o server;

server.o: server.c
	gcc ${CFLAGS} -c server.c -o server.o;

clean:
	\rm -rf *.o server  *~
