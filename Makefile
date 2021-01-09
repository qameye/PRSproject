CFLAGS = -g -Wall -DDEBUG
LDLAGS = -g -Wall -DDEBUG

all: main

main: server1.o server1_sPF.o server1_sPF_grosFichier.o
	gcc ${LDLAGS} server1.o -o server1;
	gcc ${LDLAGS} server1_sPF.o -o server1_sPF;
	gcc ${LDLAGS} server1_sPF_grosFichier.o -o server1_sPF_grosFichier;

server.o: server.c server1_sPF.c server1_sPF_grosFichier.c
	gcc ${CFLAGS} -c server1.c -o server1.o;
	gcc ${CFLAGS} -c server1_sPF.c -o server1_sPF.o;
	gcc ${CFLAGS} -c server1_sPF_grosFichier.c -o server1_sPF_grosFichier.o;

clean:
	\rm -rf *.o server1  *~
	\rm -rf *.o server1_sPF  *~
	\rm -rf *.o server1_sPF_grosFichier  *~
