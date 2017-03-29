CC=$(CROSS_COMPILE)gcc

PREFIX=transport_buidler_
TOP_DIR=../

CFLAGS=-Wall -fno-omit-frame-pointer -I$(TOP_DIR) -g

# CFLAGS+=-DPB_LOCK_DEBUG
CFLAGS+=-DTIME_DEBUG
CFLAGS+=-DQUEUE_DEBUG
# CFLAGS+=-DDYNAMIC_IDENTITY

# CFLAGS=-Werror

FILES:=$(patsubst %.c,$(PREFIX)%.o,$(wildcard *.c))

#CFLAGS+=-pg

all: compile

compile: $(FILES)

$(PREFIX)%.o: $(TOP_DIR)obj Makefile
	$(CC) $(CFLAGS) $(LDFLAGS) -c $*.c -o $(TOP_DIR)obj/$*.o

$(TOP_DIR)obj:
	mkdir -p $(TOP_DIR)obj

clean:
	rm -f ../obj
