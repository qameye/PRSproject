CFLAGS = -g -Wall -DDEBUG
LDLAGS = -g -Wall -DDEBUG

all: main

main: server.o
	gcc ${LDLAGS} serveur1-UDPower.o -o serveur1-UDPower;
	gcc ${LDLAGS} serveur2-UDPower.o -o serveur2-UDPower;

server.o: serveur1-UDPower.c serveur2-UDPower.c
	gcc ${CFLAGS} -c serveur1-UDPower.c -o serveur1-UDPower.o;
	gcc ${CFLAGS} -c serveur2-UDPower.c -o serveur2-UDPower.o;

clean:
	\rm -rf *.o serveur1-UDPower  *~
	\rm -rf *.o serveur2-UDPower  *~
