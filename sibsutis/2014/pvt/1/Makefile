CFLAGS += -Wall -Werror -g
#CFLAGS += -DDEBUG

all:
	gcc $(CFLAGS) main.c -o main
	gcc $(CFLAGS) main_pth.c -o main_pth -lpthread

clean:
	rm -f *.exe