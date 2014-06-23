SOURCE_DIR=src/
INCLUDE_DIR=$(SOURCE_DIR)include/
LIB_DIR=lib/
OBJ_DIR=obj/

OBJ:=$(wildcard $(OBJ_DIR)*.o)

#ASN_MODULE=MyModule.asn1
#ASN_MODULE=SMARTI.asn
ASN_MODULE=pong.asn

all: create_src create_lib

create_src: asn_translate
	mv *.c $(SOURCE_DIR)
	mv *.h $(INCLUDE_DIR)
	mv *.sample $(SOURCE_DIR)
	$(MAKE) CC=$(CC) -C src/

create_lib:
	ar rc $(LIB_DIR)libasn.a $(OBJ)

asn_translate:
	./asn1c $(ASN_MODULE)

clean:
	$(MAKE) -C src/ clean
	rm -f $(OBJ_DIR)*.o
	rm -f $(LIB_DIR)*.a
