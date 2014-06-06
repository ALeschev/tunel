
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
BINNAME = cmd_test

CORE_PATH=./
SRC_IPCLIB=../libipc

LFLAGS  = -Wall -pedantic -O

LIB     = 

OBJ_DIR = ./obj
OBJS =  $(OBJ_DIR)/main.o $(OBJ_DIR)/command_proc.o

all: copy

copy:
	@mkdir -p $(OBJ_DIR)
	@+make $(BINNAME)
	@mkdir -p $(ARCH)
	mv $(BINNAME) ./$(ARCH)/$(BINNAME)

$(BINNAME): $(LIBS) $(OBJS) 
	$(CC) $(LFLAGS)  -o $@ $(OBJS) $(LIB)

$(OBJ_DIR)/main.o: $(CORE_PATH)/main.c $(CORE_PATH)/command_proc.c $(CORE_PATH)/command_proc.h
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/command_proc.o: $(CORE_PATH)/command_proc.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(OBJ_DIR) ./$(ARCH)
