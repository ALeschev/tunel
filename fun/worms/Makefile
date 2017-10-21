CC=gcc
CFLAGS=-c -Wall
LIB=-lcurses
SOURCES=main.c worms.c helper.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=worms

all: $(SOURCES) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS)
		$(CC) $(OBJECTS) $(LIB) -o $@

.o: $(SOURCES)
		$(CC) $(CFLAGS) $<

clean:
		rm -f *.o *.a $(EXECUTABLE)
