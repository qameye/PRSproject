CFLAGS = -g -Wall -DDEBUG
LDLAGS = -g -Wall -DDEBUG

all: main

main: server1.o server1_sPF.o
	gcc ${LDLAGS} server1.o -o server1;
	gcc ${LDLAGS} server1_sPF.o -o server1_sPF;

server.o: server.c server1_sPF.c
	gcc ${CFLAGS} -c server1.c -o server1.o;
	gcc ${CFLAGS} -c server1_sPF.c -o server1_sPF.o;

clean:
	\rm -rf *.o server1  *~
	\rm -rf *.o server1_sPF  *~
