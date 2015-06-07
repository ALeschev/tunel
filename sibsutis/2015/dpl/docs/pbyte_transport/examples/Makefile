CC=$(CROSS_COMPILE)gcc
CXX=$(CROSS_COMPILE)g++

ARCH=x86_64

# ifeq ($(CROSS_COMPILE),x86_64-linux-gnu-)
# ARCH=x86_64
# endif

ifeq ($(CROSS_COMPILE),arm-mv5sft-linux-gnueabi-)
ARCH=mv
endif

ifeq ($(CROSS_COMPILE),arm-marvell-linux-gnueabi-)
ARCH=2016
endif

CFLAGS = -g -I ../../.build/$(ARCH)/usr/local/include  -I ../include/
#CFLAGS += -Werror
LDFLAGS = -L../../.build/$(ARCH)/usr/local/lib/ -L../../lib/$(ARCH)/
LIB=-lpbyte -lzmq -lpthread -lrt

#CFLAGS+= -pg

.PHONY: all clean

all: dclient rserver

dclient: dclient.c
	$(CXX) $(CFLAGS) $(LDFLAGS) dclient.c -o dclient $(LIB)

rserver: rserver.c
	$(CXX) $(CFLAGS) $(LDFLAGS) rserver.c -o rserver $(LIB)

clean:
	rm -rf rserver dclient