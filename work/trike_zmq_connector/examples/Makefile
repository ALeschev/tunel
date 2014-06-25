PREFIX=trike_example_
TOP_DIR=../
CFLAGS=-I $(TOP_DIR)include/ -I ../../zmq_transport_test/asn_test/asn/src/include
LDFLAGS=$(TOP_DIR)lib/libtzmq.a -L $(TOP_DIR)lib/ ../../zmq_transport_test/asn_test/asn/lib/libasn.a -lzmq -lprotobuf-c -lpthread
FILES:=$(patsubst %.c,$(PREFIX)%.o,$(wildcard *.c))

CFLAGS+=-DERL_TEST

all: compile

compile: $(FILES)

$(PREFIX)%.o:
	$(CC) $*.c $(CFLAGS) $(LDFLAGS) -o $*.out -g

clean:
	rm -f *.out
