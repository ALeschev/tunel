BIN_NAME=v4l2grab
BIN_DIR=bin
CC=gcc

ifeq ($(ARCH), mv)
    CC=arm-mv5sft-linux-gnueabi-gcc
endif

all: chkdir
	$(CC) -I../libs/libjpeg -L../libs/libjpeg/.libs tojpg.c -o $(BIN_DIR)/$(BIN_NAME) -Wall -ljpeg -DIO_READ -DIO_MMAP -DIO_USERPTR -ljpeg


chkdir:
	mkdir -p $(BIN_DIR)

