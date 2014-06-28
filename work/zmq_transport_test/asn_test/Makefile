ASN_DIR=asn/
SOURCE_DIR=$(ASN_DIR)core/
INCLUDE_DIR=$(SOURCE_DIR)/include/
LIB_DIR=$(ASN_DIR)/$(ARCH)/lib/

CFLAGS=-I $(INCLUDE_DIR) -I. -g
LDFLAGS=-L $(LIB_DIR)

LIB = -lasn

all: clean build_lib compile

compile: Seize_cede_test.c asn_der_msg.c asn_der_msg.h
	$(CC) $(CFLAGS) $(LDFLAGS) asn_der_msg.c Seize_cede_test.c -o Seize_cede_test $(LIB)

build_lib:
	$(MAKE) CC=$(CC) -C asn/ asn_translate
	$(MAKE) CC=$(CC) -C asn/ create_lib

clean:
	$(MAKE) CC=$(CC) -C asn/ clean
	rm -f *.out
