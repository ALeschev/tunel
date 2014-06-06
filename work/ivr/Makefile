
CROSS_COMPILE=

CC=$(CROSS_COMPILE)gcc
AR=$(CROSS_COMPILE)ar
STRIP=$(CROSS_COMPILE)strip

OPTIMIZE= -O2 -fno-strength-reduce -fsigned-char
OPTIMIZE2= -fomit-frame-pointer
DEBUG_OPT= -g
CFLAGS  = -c -gdwarf-2 -Wall $(DEBUG_OPT) $(DEBUG) $(OPTIMIZE) -DFLAG_LINUX -D_LINUX_
CFLAGS += -DHAVE_PTHREAD -DHAVE_SEMAPHORE_H
CFLAGS += -D_U16_INDEX_
#CFLAGS += -DMEMDBG
#CFLAGS += -DIPROUTE_DIFFERENT_TABLES
BINNAME = ivr


ifeq ($(CROSS_COMPILE),ppc_4xxFP-)
CFLAGS += -mpowerpc -Werror
ARCH=ppc
endif

ifeq ($(CROSS_COMPILE),arm-mv5sft-linux-gnueabi-)
ARCH=mv
CFLAGS += -D_SMG1016M_ -Werror

ifdef USE_DSA
CFLAGS += -DUSE_DSA
endif

endif


ifeq ($(CROSS_COMPILE),arm-marvell-linux-gnueabi-)
ARCH=2016
CFLAGS += -DBOARD_SMG2016
endif


CORE_PATH=./core
INC_PATH=$(CORE_PATH)/include
SRC_IPCLIB=../libipc

INC=-I$(INC_PATH)

LIB_DIR = ./$(ARCH)/lib
LFLAGS  = -Wall -pedantic -O -pthread
LFLAGS += -L$(LIB_DIR)

LIB     = -lpthread -lcrypt -lrt -lIPC

LIB_IPC = $(LIB_DIR)/libIPC.a
LIBS    = $(LIB_IPC)


OBJ_DIR = ./$(ARCH)/obj
OBJS =  $(OBJ_DIR)/main.o $(OBJ_DIR)/methods.o $(OBJ_DIR)/caller_tree.o

all: copy

copy:
#	@mkdir -p $(LIB_DIR)
#	@mkdir -p $(OBJ_DIR)
#	touch $(CORE_PATH)/making.c
	@+make $(BINNAME)
	mv $(BINNAME) ./$(ARCH)/$(BINNAME)

$(BINNAME): $(LIBS) $(OBJS) 
	$(CC) $(LFLAGS)  -o $@ $(OBJS) $(LIB)
	cp $@ $(ARCH)/$(BINNAME)_uns
	$(STRIP) $@

$(OBJ_DIR)/main.o: $(CORE_PATH)/main.c $(CORE_PATH)/methods.c $(INC_PATH)/config_description.h $(INC_PATH)/methods.h $(INC_PATH)/ivr_def.h \
	               $(INC_PATH)/header.h $(INC_PATH)/object_info.h $(INC_PATH)/handlers.h $(INC_PATH)/sysmodules.h  $(INC_PATH)/ivr_proc.h \
	               $(INC_PATH)/object_info.h $(INC_PATH)/uemf.h $(INC_PATH)/mqueue.h $(INC_PATH)/ivr_caller_tree.h
	$(CC) $(CFLAGS) $(INC) $< -o $@

$(OBJ_DIR)/methods.o: $(CORE_PATH)/methods.c $(CORE_PATH)/caller_tree.c $(INC_PATH)/methods.h $(INC_PATH)/ivr_proc.h $(INC_PATH)/header.h $(INC_PATH)/object_info.h \
	                  $(INC_PATH)/handlers.h $(INC_PATH)/sysmodules.h $(INC_PATH)/config_description.h \
	                  $(INC_PATH)/object_info.h $(INC_PATH)/uemf.h $(INC_PATH)/mqueue.h $(INC_PATH)/ivr_caller_tree.h
	$(CC) $(CFLAGS) $(INC) $< -o $@

$(OBJ_DIR)/caller_tree.o: $(CORE_PATH)/caller_tree.c $(INC_PATH)/ivr_caller_tree.h
	$(CC) $(CFLAGS) $(INC) $< -o $@

$(LIB_IPC): LIBIPC

LIBIPC:
	@+make clean -C $(SRC_IPCLIB)/ 
	@+make -C $(SRC_IPCLIB)/
	cp $(SRC_IPCLIB)/$(ARCH)/libIPC.a $(LIB_DIR)/

clean:
	rm -f $(OBJ_DIR)/*.[oas] *.[oas] ./$(ARCH)/$(BINNAME) $(LIB_DIR)/*
