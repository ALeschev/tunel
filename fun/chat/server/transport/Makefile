CFLAGS= -g -fno-omit-frame-pointer

ifeq ($(CROSS_COMPILE),x86_64-linux-gnu-)
HOST=x86_64-linux-gnu
endif

all: build_pbyte

build_pbyte:
	$(MAKE) -C pbyte_transport

distclean: clean_pbyte clean_trike clean_deps
	rm -fr .build

clean_pbyte:
	$(MAKE) -C pbyte_transport clean

distclean_pbyte: clean_pbyte
	rm -rf .build/$(ARCH)
