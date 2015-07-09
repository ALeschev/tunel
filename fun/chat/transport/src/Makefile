CC=$(CROSS_COMPILE)gcc

PREFIX=pbyte_buidler_

TOP_DIR=..
INCLUDE=$(TOP_DIR)/include
LOGGER_DIR=../../logger

CFLAGS+=-g -Wall -fno-omit-frame-pointer -I ./ -I $(INCLUDE) -I $(LOGGER_DIR)
#CFLAGS+=-Werror

FILES:=$(patsubst %.c,$(PREFIX)%.o,$(wildcard *.c))

#CFLAGS+=-pg

all: compile

compile: $(FILES)

$(PREFIX)%.o: $(TOP_DIR)/obj Makefile
	$(CC) $(CFLAGS) $(LDFLAGS) -c $*.c -o $(TOP_DIR)/obj/$*.o

$(TOP_DIR)/obj:
	mkdir -p $(TOP_DIR)/obj

clean:
	rm -f ../obj
