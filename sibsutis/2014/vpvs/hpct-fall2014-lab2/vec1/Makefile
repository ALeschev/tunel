prog := vec
prog_objs := vec.o hpctimer.o

CC := gcc
CFLAGS := -Wall -g -O2 -msse3 
LDFLAGS := -lm

.PHONY: all clean

all: $(prog)

$(prog): $(prog_objs)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

vec.o: vec.c
hpctimer.o: hpctimer.c hpctimer.h

clean:
	@rm -rf *.o $(prog)
