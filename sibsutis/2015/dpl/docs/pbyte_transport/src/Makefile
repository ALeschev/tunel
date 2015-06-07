CC=$(CROSS_COMPILE)gcc

PREFIX=pbyte_buidler_
TOP_DIR=../

CFLAGS=-Wall -Werror -fno-omit-frame-pointer -I$(TOP_DIR)../.build/$(ARCH)/usr/local/include \
       -I$(TOP_DIR)../.build/$(ARCH)/usr/local/include/google -I ./ -I $(TOP_DIR)include/ -g

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
