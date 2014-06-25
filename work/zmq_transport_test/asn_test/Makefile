ASN_DIR=asn/
SOURCE_DIR=$(ASN_DIR)src/
INCLUDE_DIR=$(SOURCE_DIR)include/
LIB_DIR=$(ASN_DIR)lib/

CFLAGS=-I $(INCLUDE_DIR) -I.
LDFLAGS=-L $(LIB_DIR)

LIB = -lasn

all: clean build_lib compile

compile: Seize_cede_test.c asn_der_msg.c asn_der_msg.h
	$(CC) $(CFLAGS) $(LDFLAGS) asn_der_msg.c Seize_cede_test.c -o Seize_cede_test $(LIB)

build_lib:
	$(MAKE) CC=$(CC) -C asn/ create_src
	$(MAKE) CC=$(CC) -C asn/ create_lib

clean:
	$(MAKE) CC=$(CC) -C asn/ clean
	rm -f *.out
