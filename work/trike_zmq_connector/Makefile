OS_NAME = $(strip $(shell uname | tr '[A-Z]' '[a-z]'))

#ifeq ($(OS_NAME),darwin)
#CC=clang
#CC_TEST=clang++
#else ifeq($(OS_NAME),linux)
#CC=gcc
#CC_TEST=g++
#endif

CFLAGS= -g -c

all:  test_compile obj_clean compile link build_examples

check: test_compile

test_compile: obj_clean
	$(MAKE) CC=$(CC_TEST) -C src/ compile

compile:
	$(MAKE) CC=$(CC) -C src/ compile

build_deps:
#	@cd lib/gtest-1.7.0 && ./configure --host=arm-mv5sft-linux-gnueabi && make
	@cd lib/zeromq-3.2.4 && ./configure --host=arm-mv5sft-linux-gnueabi --prefix=$(CURDIR)/lib/zeromq-3.2.4/ --libdir=$(CURDIR)/lib --includedir=$(CURDIR)/include && make && make install
	@cd lib/protobuf-c && ./autogen.sh && ./configure --host=arm-mv5sft-linux-gnueabi --prefix=$(CURDIR)/lib/protobuf-2.5.0/ --libdir=$(CURDIR)/lib --includedir=$(CURDIR)/include --disable-protoc && make && make install

build_examples:
	$(MAKE) CC=$(CC) -C examples/ compile

link:
	arm-mv5sft-linux-gnueabi-ar rc lib/libtzmq.a $(wildcard obj/*.o)

obj_clean:
	rm -f obj/*.o

clean:
	rm -f lib/*.a
	rm -f obj/*.o
	make -C test/ clean
	make -C examples/ clean

clean_deps:
	@cd lib/gtest-1.7.0 && make clean
	@cd lib/zeromq-3.2.4 && make clean
