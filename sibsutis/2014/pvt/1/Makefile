CFLAGS += -Wall -Werror -g
#CFLAGS += -DDEBUG
CFLGS += -I./

all:
	gcc $(CFLAGS) main.c hpctimer.c  -o main
	gcc $(CFLAGS) main_pth.c hpctimer.c -o main_pth -lpthread

clean:
	rm -f main main_pth