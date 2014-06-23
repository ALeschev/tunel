PREFIX=asn_test_
ASN_DIR=asn/
SOURCE_DIR=$(ASN_DIR)src/
INCLUDE_DIR=$(SOURCE_DIR)include/
LIB_DIR=$(ASN_DIR)lib/

CFLAGS=-I $(INCLUDE_DIR)
LDFLAGS=-L $(LIB_DIR) -lasn

all: clean build_lib compile

compile: $(patsubst %.c,$(PREFIX)%.o,$(wildcard *.c))

build_lib:
	$(MAKE) CC=$(CC) -C asn/ create_src
	$(MAKE) CC=$(CC) -C asn/ create_lib

$(PREFIX)%.o:
	$(CC) $*.c $(CFLAGS) $(LDFLAGS) -o $*.out

clean:
	$(MAKE) CC=$(CC) -C asn/ clean
	rm -f *.out
