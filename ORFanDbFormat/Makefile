CC = g++
CCFLAGS = -std=c++11 -pthread

ORFanDbFormat: formatDatabase.o hashFunctions.o
	$(CC) $(CCFLAGS) -o ORFanDbFormat formatDatabase.o hashFunctions.o

formatDatabase.o: formatDatabase.cc
	$(CC) $(CCFLAGS) -c formatDatabase.cc

hashFunctions.o:  hashFunctions.cc hashFunctions.h
	$(CC) $(CCFLAGS) -c hashFunctions.cc

clean:
	rm *.o
