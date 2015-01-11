CC = g++
CFLAGS = -Wall -lm

all: c.o c2.o
	$(CC) c.o c2.o -o c
 
c.o: c.cpp
 
c2.o: c2.cpp

clean:
	@rm -f *.o
