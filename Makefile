CC=g++
COPTIMIZE= -O2
CFLAGS= -g -Wall -pthread -MD -MP -std=c++11 $(COPTIMIZE)

default: all

all: test

test: TCPOverUDP.o test.o
	$(CC) -o $@ $^ $(CFLAGS) 

clean: rm -rf *.o
