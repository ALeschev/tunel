PREFIX=trike_example_
TOP_DIR=../
CFLAGS=-I $(TOP_DIR)include/
LDFLAGS=$(TOP_DIR)lib/libtzmq.a -L $(TOP_DIR)lib/ -lzmq -lprotobuf-c
FILES:=$(patsubst %.c,$(PREFIX)%.o,$(wildcard *.c))

all: compile

compile: $(FILES)

$(PREFIX)%.o:
	$(CC) $*.c $(CFLAGS) $(LDFLAGS) -o $*.out

clean:
	rm -f *.out
