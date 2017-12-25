CFLAGS = -Wall

SERVER = simpled
CLIENTS = ssSet ssGet ssDigest ssRun

all: $(SERVER) $(CLIENTS)

csapp.h:
	wget http://csapp.cs.cmu.edu/2e/ics2/code/include/csapp.h

csapp.c:
	wget http://csapp.cs.cmu.edu/2e/ics2/code/src/csapp.c

csapp.o: csapp.h csapp.c

clients: ssSet ssGet ssDigest ssRun

simpled: simpled.cpp csapp.o 
	g++ $(CFLAGS) simpled.cpp csapp.o -lpthread -o simpled

ssSet: ssSet.cpp csapp.o 
	g++ $(CFLAGS) ssSet.cpp csapp.o -lpthread -o ssSet

ssGet: ssGet.cpp csapp.o 
	g++ $(CFLAGS) ssGet.cpp csapp.o -lpthread -o ssGet

ssDigest: ssDigest.cpp csapp.o 
	g++ $(CFLAGS) ssDigest.cpp csapp.o -lpthread -o ssDigest

ssRun: ssRun.cpp csapp.o 
	g++ $(CFLAGS) ssRun.cpp csapp.o -lpthread -o ssRun

.PHONY: clean
clean:
	/bin/rm -rf csapp.h csapp.c *.o simpled ssSet ssGet ssDigest ssRun

