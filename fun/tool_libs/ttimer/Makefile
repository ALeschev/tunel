
CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
STRIP = $(CROSS_COMPILE)strip
AR = $(CROSS_COMPILE)ar

BUILD_DIR    = .build
SOURCE_DIR   = source
LIB_TEST_DIR = test

HOST = x86_64

# ifeq ($(CROSS_COMPILE),x86_64-linux-gnu-)
# HOST=x86_64-linux-gnu
# endif

OBJ_DIR = $(BUILD_DIR)/$(HOST)/obj
LIB_DIR = $(BUILD_DIR)/$(HOST)/lib
BIN_DIR = $(BUILD_DIR)/$(HOST)/bin

CFLAGS  = -g -Wno-write-strings
CFLAGS += -I$(SOURCE_DIR)

LDFLAGS = -L$(LIB_DIR)

LIBS    = -lttimer -lpthread

TTIMER_LIB_TARGET = $(LIB_DIR)/libttimer.a
TTIMER_LIB_DEPS   = $(SOURCE_DIR)/ttimer.c
TTIMER_LIB_OBJ    = $(OBJ_DIR)/ttimer.o

TTIMER_TEST_TARGET = $(BIN_DIR)/ttest
TTIMER_TEST_DEPS   = $(LIB_TEST_DIR)/ttest.c
TTIMER_TEST_OBJ    = $(OBJ_DIR)/ttest.o

.PHONY:build_dir

all: build_dir lib test

lib: build_dir $(TTIMER_LIB_TARGET)

test: build_dir lib $(TTIMER_TEST_TARGET)

build_dir:
	@mkdir -vp $(OBJ_DIR)
	@mkdir -vp $(LIB_DIR)
	@mkdir -vp $(BIN_DIR)

$(TTIMER_LIB_TARGET): $(TTIMER_LIB_OBJ)
	$(AR) rs $(TTIMER_LIB_TARGET) $(TTIMER_LIB_OBJ)

$(TTIMER_TEST_TARGET):  $(TTIMER_TEST_OBJ) $(TTIMER_LIB_OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TTIMER_TEST_OBJ) $(LIBS)
	cp $(TTIMER_TEST_TARGET) $(TTIMER_TEST_TARGET)_uns
	$(STRIP) $(TTIMER_TEST_TARGET)

$(OBJ_DIR)/ttimer.o : $(TTIMER_LIB_DEPS)
	$(CC) $(CFLAGS) -c -o $@  $<

$(OBJ_DIR)/ttest.o : $(TTIMER_TEST_DEPS)
	$(CC) $(CFLAGS) -c -o $@  $<

clean:
	rm -rf $(BUILD_DIR)/$(HOST)
