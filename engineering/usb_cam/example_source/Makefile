BIN_NAME=v4l2grab
BIN_DIR=bin

all: chkdir
	gcc tojpg.c -o $(BIN_DIR)/$(BIN_NAME) -Wall -ljpeg -DIO_READ -DIO_MMAP -DIO_USERPTR


chkdir:
	mkdir -p $(BIN_DIR)

