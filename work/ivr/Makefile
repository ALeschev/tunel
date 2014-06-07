
CROSS_COMPILE=

CC=$(CROSS_COMPILE)gcc
AR=$(CROSS_COMPILE)ar
STRIP=$(CROSS_COMPILE)strip

OPTIMIZE= -O2 -fno-strength-reduce -fsigned-char
OPTIMIZE2= -fomit-frame-pointer
DEBUG_OPT= -g
CFLAGS  = -c -gdwarf-2 -Wall $(DEBUG_OPT) $(DEBUG) $(OPTIMIZE) -DFLAG_LINUX -D_LINUX_
CFLAGS += -DHAVE_PTHREAD -DHAVE_SEMAPHORE_H
CFLAGS += -D_U16_INDEX_ -DCLI

CFLAGS += -DMQUEUE_USE_UT
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

SRC_CLILIB=../libcli
CORE_PATH=./core
INC_PATH=$(CORE_PATH)/include
SRC_IPCLIB=../libipc

INC=-I$(INC_PATH) -I$(INC_PATH)/cli/

LIB_DIR = ./$(ARCH)/lib
LFLAGS  = -Wall -pedantic -O -pthread
LFLAGS += -L$(LIB_DIR)

UT_PATH=$(CORE_PATH)/ut
UT_OBJ_PATH=$(ARCH)/obj/ut
UT_SRC_PATH=$(UT_PATH)
UT_OBJS=$(UT_OBJ_PATH)/ut_sema.o $(UT_OBJ_PATH)/ut_queues.o $(UT_OBJ_PATH)/ut_malloc.o $(UT_OBJ_PATH)/ut_thread.o
UT_CFLAGS=

LIB_UT  = $(LIB_DIR)/libut.a
LIB_CLI = $(LIB_DIR)/libcli.a
LIB_IPC = $(LIB_DIR)/libIPC.a

LIB     = -lut -lpthread -lcli -lcrypt -lrt -lIPC
LIBS    = $(LIB_UT) $(LIB_CLI) $(LIB_IPC)

OBJ_DIR = ./$(ARCH)/obj
OBJS =  $(OBJ_DIR)/main.o $(OBJ_DIR)/methods.o $(OBJ_DIR)/caller_tree.o $(OBJ_DIR)/ivr_def.o \
        $(OBJ_DIR)/consol.o

all: copy

copy:
	@mkdir -p $(ARCH)
	@mkdir -p $(LIB_DIR)
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(UT_OBJ_PATH)
	touch $(CORE_PATH)/making.c
	@+make $(BINNAME)
	mv $(BINNAME) ./$(ARCH)/$(BINNAME)

$(BINNAME): $(LIBS) $(OBJS) 
	$(CC) $(LFLAGS)  -o $@ $(OBJS) $(LIB)
	cp $@ $(ARCH)/$(BINNAME)_uns
	$(STRIP) $@

UT_HEADERS = $(INC_PATH)/ut/ut_sema.h $(INC_PATH)/ut/ut_queues.h $(INC_PATH)/ut/ut_port.h  $(INC_PATH)/ut/ut_thread.h

$(OBJ_DIR)/main.o: $(CORE_PATH)/main.c $(CORE_PATH)/methods.c $(UT_HEADERS) $(INC_PATH)/config_description.h $(INC_PATH)/methods.h $(INC_PATH)/ivr_def.h \
	               $(INC_PATH)/header.h $(INC_PATH)/object_info.h $(INC_PATH)/handlers.h $(INC_PATH)/sysmodules.h  $(INC_PATH)/ivr_proc.h \
	               $(INC_PATH)/object_info.h $(INC_PATH)/uemf.h $(INC_PATH)/mqueue.h $(INC_PATH)/ivr_caller_tree.h
	$(CC) $(CFLAGS) $(INC) $< -o $@

$(OBJ_DIR)/consol.o: $(CORE_PATH)/consol.c $(UT_HEADERS) $(INC_PATH)/mqueue.h
	$(CC) $(CFLAGS) $(INC) $< -o $@

$(OBJ_DIR)/methods.o: $(CORE_PATH)/methods.c $(CORE_PATH)/caller_tree.c $(INC_PATH)/methods.h $(INC_PATH)/ivr_proc.h $(INC_PATH)/header.h $(INC_PATH)/object_info.h \
	                  $(INC_PATH)/handlers.h $(INC_PATH)/sysmodules.h $(INC_PATH)/config_description.h \
	                  $(INC_PATH)/object_info.h $(INC_PATH)/uemf.h $(INC_PATH)/mqueue.h $(INC_PATH)/ivr_caller_tree.h
	$(CC) $(CFLAGS) $(INC) $< -o $@

$(OBJ_DIR)/caller_tree.o: $(CORE_PATH)/caller_tree.c $(INC_PATH)/ivr_caller_tree.h
	$(CC) $(CFLAGS) $(INC) $< -o $@

$(OBJ_DIR)/ivr_def.o: $(CORE_PATH)/ivr_def.c $(INC_PATH)/ivr_def.h
	$(CC) $(CFLAGS) $(INC) $< -o $@

$(LIB_UT): $(UT_OBJS)
	$(AR) r $@ $(UT_OBJ_PATH)/*.o
	@echo "Compiled Utilities "
	@echo "++++++++++++++++++ "
	@echo

$(UT_OBJ_PATH)/%.o: $(UT_SRC_PATH)/%.c
	$(CC)  $(CFLAGS) $(UT_CFLAGS) $(INC) $<  -o $@

$(LIB_CLI): LIBCLI

LIBCLI:
	@+make -C $(SRC_CLILIB)/
	cp $(SRC_CLILIB)/$(ARCH)/libcli.a $(LIB_DIR)/

$(LIB_IPC): LIBIPC

LIBIPC:
	@+make clean -C $(SRC_IPCLIB)/ 
	@+make -C $(SRC_IPCLIB)/
	cp $(SRC_IPCLIB)/$(ARCH)/libIPC.a $(LIB_DIR)/

clean:
	rm -f $(OBJ_DIR)/*.[oas] *.[oas] ./$(ARCH)/$(BINNAME) $(LIB_DIR)/* $(UT_OBJ_PATH)/*.o

cleanlib:
	rm -f $(LIB_DIR)/*

cleanall: clean cleanlib
