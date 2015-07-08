AR=$(CROSS_COMPILE)ar

LIB_DIR = ../lib/$(ARCH)

all: link

link: compile
	mkdir -p $(LIB_DIR)
	$(AR) rc $(LIB_DIR)/libpbyte.a $(wildcard obj/*.o)

compile:
	$(MAKE) -C src/ compile

clean:
	rm -rf ./obj $(LIB_DIR)/libpbyte.a
