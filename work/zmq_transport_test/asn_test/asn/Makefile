
CC=$(CROSS_COMPILE)gcc
AR=$(CROSS_COMPILE)ar

CORE_PATH=core/
INCLUDE_DIR=$(CORE_PATH)include/

ASN_MODULE=SMARTI.asn

OPTIMIZE= -O2 -fno-strength-reduce -fsigned-char
OPTIMIZE2= -fomit-frame-pointer
DEBUG_OPT= -g
CFLAGS = -c -gdwarf-2 -Wall $(DEBUG_OPT) $(DEBUG) $(OPTIMIZE) -Wno-pointer-sign

CFLAGS+=-I $(INCLUDE_DIR)

CFLAGS+=-I ../../../asn1c/skeletons/

ifeq ($(CROSS_COMPILE),ppc_4xxFP-)
CFLAGS += -mpowerpc
ARCH=ppc
endif

ifeq ($(CROSS_COMPILE),arm-mv5sft-linux-gnueabi-)
ARCH=mv
endif

ifeq ($(CROSS_COMPILE),arm-marvell-linux-gnueabi-)
ARCH=2016
CFLAGS += -Wno-unused-but-set-variable -fpartial-inlining
endif

LIB_DIR=$(ARCH)/lib/
OBJ_DIR=$(ARCH)/obj/

all: asn_translate

asn_translate:
	./asn1c -S ../../../asn1c/skeletons/ $(ASN_MODULE)
	@mkdir -p $(CORE_PATH)
	@mkdir -p $(INCLUDE_DIR)
	@mkdir -p $(OBJ_DIR)
	@mv *.c $(CORE_PATH)
	@mv *.h $(INCLUDE_DIR)
	@rm -f converter-sample.c
	@rm -f $(SOURCE_DIR)*.sample

create_lib: create_obj
	@mkdir -p $(LIB_DIR)
	$(AR) rc $(LIB_DIR)libasn.a $(wildcard $(OBJ_DIR)*.o)

create_obj: $(patsubst %.c,%.o,$(wildcard $(CORE_PATH)*.c))
#	@+make $(J8) CC=$(CC) FLAGS=$(FLAGS) create_lib

%.o: %.c
	$(CC) $(J8) $(CFLAGS) $< $(FLAGS) -o $@
	@mv $@ $(OBJ_DIR)

clean:
	@rm -f $(SOURCE_DIR)*.c
	@rm -rf $(INCLUDE_DIR)
	@rm -rf $(ARCH)
	@rm -rf $(OBJ_DIR)
	@rm -rf $(CORE_PATH)
