CC=$(CROSS_COMPILE)gcc
AR=$(CROSS_COMPILE)ar
ARCH=x68_64

LIB_DIR = ./lib/$(ARCH)

all: link

link: make_lib build
	mkdir -p $(LIB_DIR)
	$(AR) rc $(LIB_DIR)/libtransport.a $(wildcard obj/*.o)

make_lib:
	$(MAKE) -C transport/ compile

build: $(LIB_DIR)/libtransport.a dispatcher.c
	$(CC) dispatcher.c -o dispatcher $(LIB_DIR)/libtransport.a

clean: exam_clean
	rm -rf ./obj $(LIB_DIR)/libtransport.a
