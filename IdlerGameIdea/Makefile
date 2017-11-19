CC=g++
CFLAGS=-c -O2

all: ThreadIdler

ThreadIdler: binary_semaphore.o ThreadIdler.o
	$(CC) binary_semaphore.o ThreadIdler.o -o ThreadIdler -lpthread 

binary_semaphore.o: binary_semaphore.cpp
	$(CC) $(CFLAGS) binary_semaphore.cpp

ThreadIdler.o: ThreadIdler.cpp
	$(CC) $(CFLAGS) ThreadIdler.cpp

clean:
	/bin/rm -f ThreadIdler *.o *.gz

run:
	./ThreadIdler