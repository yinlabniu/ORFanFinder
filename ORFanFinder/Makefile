CC = g++
CCFLAGS = -std=c++11 -pthread

ORFanFinder: main.o DBContainer.o TaxTree.o OrfProcessor.o TaxNode.o
	$(CC) $(CCFLAGS) -o ORFanFinder main.o DBContainer.o TaxTree.o OrfProcessor.o TaxNode.o

main.o: main.cc
	$(CC) $(CCFLAGS) -c main.cc

DBContainer.o:  DBContainer.cc DBContainer.h
	$(CC) $(CCFLAGS) -c DBContainer.cc

TaxNode.o: TaxNode.cc TaxNode.h
	$(CC) $(CCFLAGS) -c TaxNode.cc

TaxTree.o: TaxTree.cc TaxTree.h
	$(CC) $(CCFLAGS) -c TaxTree.cc

OrfProcessor.o: OrfProcessor.cc OrfProcessor.h
	$(CC) $(CCFLAGS) -c OrfProcessor.cc

clean:
	rm *.o
