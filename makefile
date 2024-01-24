CC = gcc
DEBUG = -g
CFLAGS = -Wall -lpthread -lrt -std=c99 -pedantic -c $(DEBUG)
LFLAGS = -Wall -lpthread -lrt -std=c99 -pedantic $(DEBUG)

Simulator : OS_SimDriver.o simulator.o metadataops.o configops.o stringUtil.o simtimer.o
	$(CC) $(LFLAGS) OS_SimDriver.o simulator.o metadataops.o configops.o stringUtil.o simtimer.o -o sim04

OS_SimDriver.o : OS_SimDriver.c
	$(CC) $(CFLAGS) OS_SimDriver.c

simulator.o : simulator.c simulator.h
	$(CC) $(CFLAGS) simulator.c

metadataops.o : metadataops.c metadataops.h
	$(CC) $(CFLAGS) metadataops.c

configops.o : configops.c configops.h
	$(CC) $(CFLAGS) configops.c

stringUtil.o : stringUtil.c stringUtil.h
	$(CC) $(CFLAGS) stringUtil.c

simtimer.o : simtimer.c simtimer.h
	$(CC) $(CFLAGS) simtimer.c

clean:
	\rm *.o sim04
