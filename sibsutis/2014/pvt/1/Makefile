CFLAGS += -Wall -g
#CFLAGS += -DDEBUG

#CFLAGS += -DTHREAD_2
CFLAGS += -DTHREAD_4
#CFLAGS += -DTHREAD_6
#CFLAGS += -DTHREAD_8

all:
	gcc $(CFLAGS) main.c hpctimer.c  -o main
	gcc $(CFLAGS) main_pth.c hpctimer.c -o main_pth -lpthread

clean:
	rm -f main main_pth