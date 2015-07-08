CFLAGS= -g -fno-omit-frame-pointer

ifeq ($(CROSS_COMPILE),x86_64-linux-gnu-)
HOST=x86_64-linux-gnu
endif

ifeq ($(CROSS_COMPILE),arm-mv5sft-linux-gnueabi-)
HOST=arm-mv5sft-linux-gnueabi
endif

ifeq ($(CROSS_COMPILE),arm-marvell-linux-gnueabi-)
HOST=arm-marvell-linux-gnueabi
endif

BUILD_DIR=$(shell pwd)/.build/$(ARCH)
LIB_DIR=$(shell pwd)/lib

all: build_deps build_pbyte build_trike

build_pbyte:
	$(MAKE) -C pbyte_transport

build_trike:
	$(MAKE) -C trike_transport

build_deps: build_zmq build_protobuf_c

build_zmq: configure_zmq
	cd $(BUILD_DIR)/.zmq && make && make install DESTDIR=$(BUILD_DIR)

configure_zmq:
	mkdir -p $(BUILD_DIR)/.zmq && \
	cd $(BUILD_DIR)/.zmq && \
	$(LIB_DIR)/zeromq-4.1.2/configure --host=$(HOST) --target=$(HOST) CC=$(HOST)-gcc STRIP=$(HOST)-strip CXX=$(HOST)-g++ CXXFLAGS="$(CFLAGS) -DZMQ_SIGNALS_IN_THREAD" CFLAGS="$(CFLAGS) -DZMQ_SIGNALS_IN_THREAD" --disable-shared --without-libsodium

build_protobuf: configure_protobuf
	cd $(BUILD_DIR)/.protobuf && \
	make && make install DESTDIR=$(BUILD_DIR)

configure_protobuf:
	mkdir -p $(BUILD_DIR)/.protobuf && \
	cd $(BUILD_DIR)/.protobuf && \
	$(LIB_DIR)/protobuf-2.6.0/configure CFLAGS="$(CFLAGS)" --disable-maintainer-mode --disable-shared

build_protobuf_c: build_protobuf configure_protobuf_c
	cd $(BUILD_DIR)/.protobuf_c && \
	make && make install DESTDIR=$(BUILD_DIR)

configure_protobuf_c: build_protobuf $(LIB_DIR)/protobuf-c/configure
	mkdir -p $(BUILD_DIR)/.protobuf_c && \
	cd $(BUILD_DIR)/.protobuf_c && \
	$(LIB_DIR)/protobuf-c/configure --host=$(HOST) --target=$(HOST) CC=$(HOST)-gcc STRIP=$(HOST)-strip CXX=$(HOST)-g++ CFLAGS="$(CFLAGS)" --disable-shared --disable-protoc \
	                protobuf_CFLAGS="$(CFLAGS) -I$(BUILD_DIR)/usr/local/include" \
	                protobuf_LIBS="-L$(BUILD_DIR)/usr/local/lib" \
	                PKG_CONFIG_LIBDIR="$(BUILD_DIR)/usr/local/lib"

$(LIB_DIR)/protobuf-c/configure:
	cd $(LIB_DIR)/protobuf-c && ./autogen.sh

distclean: clean_pbyte clean_trike clean_deps
	rm -fr .build

clean_pbyte:
	$(MAKE) -C pbyte_transport clean

clean_trike:
	$(MAKE) -C trike_transport clean

distclean_pbyte: clean_pbyte
	rm -rf .build/$(ARCH)

distclean_trike: clean_trike
	rm -rf .build/$(ARCH)

clean_deps: clean_zmq clean_protobuf clean_protobuf_c

clean_zmq:
ifneq ("$(wildcard $(BUILD_DIR)/.zmq/Makefile)","")
	cd $(BUILD_DIR)/.zmq && make distclean
endif
ifneq ("$(wildcard $(BUILD_DIR)/.zmq)","")
	rm -fr $(BUILD_DIR)/.zmq
endif

clean_protobuf:
ifneq ("$(wildcard $(BUILD_DIR)/.protobuf/Makefile)","")
	cd $(BUILD_DIR)/.protobuf && make distclean
endif
ifneq ("$(wildcard $(BUILD_DIR)/.protobuf)","")
	rm -fr $(BUILD_DIR)/.protobuf
endif

clean_protobuf_c:
ifneq ("$(wildcard $(BUILD_DIR)/.protobuf_c/Makefile)","")
	cd $(BUILD_DIR)/.protobuf_c && make distclean
endif
ifneq ("$(wildcard $(BUILD_DIR)/.protobuf_c)","")
	rm -fr $(BUILD_DIR)/.protobuf_c
endif
