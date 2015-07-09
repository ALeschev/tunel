AR=$(CROSS_COMPILE)ar
CFLAGS= -g -fno-omit-frame-pointer

ifeq ($(CROSS_COMPILE),x86_64-linux-gnu-)
HOST=x86_64-linux-gnu
endif

ZMQ_DIR   = $(shell pwd)/./zeromq-4.1.2
BUILD_DIR = $(shell pwd)/.build/$(ARCH)

all: link

link: compile
	mkdir -p $(BUILD_DIR)/.pbyte
	$(AR) rc $(BUILD_DIR)/.pbyte/libpbyte.a $(wildcard obj/*.o)

compile:
	$(MAKE) -C src/ compile

build_zmq: configure_zmq
	cd $(BUILD_DIR)/.zmq && make && make install DESTDIR=$(BUILD_DIR)

configure_zmq:
	mkdir -p $(BUILD_DIR)/.zmq && \
	cd $(BUILD_DIR)/.zmq && \
	$(ZMQ_DIR)/configure --host=$(HOST) --target=$(HOST) \
                         CC=$(HOST)-gcc STRIP=$(HOST)-strip CXX=$(HOST)-g++ \
                         CXXFLAGS="$(CFLAGS) -DZMQ_SIGNALS_IN_THREAD" \
                         CFLAGS="$(CFLAGS) -DZMQ_SIGNALS_IN_THREAD" \
                         --disable-shared --without-libsodium

clean_zmq:
ifneq ("$(wildcard $(BUILD_DIR)/.zmq/Makefile)","")
	cd $(BUILD_DIR)/.zmq && make distclean
endif
ifneq ("$(wildcard $(BUILD_DIR)/.zmq)","")
	rm -fr $(BUILD_DIR)/.zmq
endif

clean:
	rm -rf ./obj $(BUILD_DIR)/.pbyte/libpbyte.a

distclean: clean clean_zmq
	rm -rf $(BUILD_DIR) $(BUILD_DIR)
