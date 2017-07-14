
all: rmall
	gcc -g -O0 --no-omit-frame-pointer -Wall live.c -o live -lncurses
	gcc -g -O0 --no-omit-frame-pointer -Wall colors.c -o colors -lncurses

rmall:
	rm -f live colors