AR=$(CROSS_COMPILE)ar

LIB_DIR = ../lib/$(ARCH)

all: link

example: exam_clean
	$(MAKE) -C examples

link: compile
	mkdir -p $(LIB_DIR)
	$(AR) rc $(LIB_DIR)/libpbyte.a $(wildcard obj/*.o)

compile:
	$(MAKE) -C src/ compile

exam_clean:
	$(MAKE) -C examples clean

clean: exam_clean
	rm -rf ./obj $(LIB_DIR)/libpbyte.a
