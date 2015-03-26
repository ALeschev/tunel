CC := gcc
CFLAGS := -O0 -Wall -g
LDFLAGS :=

.PHONY: all clean

all: worst best

worst: worst_case.c hpctimer.c
	$(CC) $(CFLAGS) worst_case.c hpctimer.c -o worst $(LDFLAGS)

best: best_case.c hpctimer.c
	$(CC) $(CFLAGS) best_case.c hpctimer.c -o best $(LDFLAGS)

clean:
	@rm -rf *.o best worst
