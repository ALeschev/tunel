PREFIX=trike_buidler_
TOP_DIR=../
CFLAGS=-I $(TOP_DIR)lib/gtest-1.7.0/include -I ./ -I $(TOP_DIR)include/
FILES:=$(patsubst %.c,$(PREFIX)%.o,$(wildcard *.c))

all: compile

compile: $(FILES)

$(PREFIX)%.o:
	$(CC) $(CFLAGS) $(LDFLAGS) -c $*.c  -o $(TOP_DIR)obj/$*.o -g

clean:
	rm *.o
