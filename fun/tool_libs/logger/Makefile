
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

CFLAGS  = -g -Wno-write-strings -Wall -Werror
CFLAGS += -I$(SOURCE_DIR)/include

LDFLAGS = -L$(LIB_DIR)

LIBS    = -llog

LOGGER_LIB_TARGET = $(LIB_DIR)/liblog.a
LOGGER_LIB_DEPS   = $(SOURCE_DIR)/logger.c
LOGGER_LIB_OBJ    = $(OBJ_DIR)/logger.o

LOGGER_TEST_TARGET = $(BIN_DIR)/ltest
LOGGER_TEST_DEPS   = $(LIB_TEST_DIR)/ltest.c
LOGGER_TEST_OBJ    = $(OBJ_DIR)/ltest.o

.PHONY:build_dir

all: build_dir lib test

lib: build_dir $(LOGGER_LIB_TARGET)

test: build_dir lib $(LOGGER_TEST_TARGET)

build_dir:
	@mkdir -vp $(OBJ_DIR)
	@mkdir -vp $(LIB_DIR)
	@mkdir -vp $(BIN_DIR)

$(LOGGER_LIB_TARGET): $(LOGGER_LIB_OBJ)
	$(AR) rs $(LOGGER_LIB_TARGET) $(LOGGER_LIB_OBJ)

$(LOGGER_TEST_TARGET):  $(LOGGER_TEST_OBJ) $(LOGGER_LIB_OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(LOGGER_TEST_OBJ) $(LIBS)
	cp $(LOGGER_TEST_TARGET) $(LOGGER_TEST_TARGET)_uns
	$(STRIP) $(LOGGER_TEST_TARGET)

$(OBJ_DIR)/logger.o : $(LOGGER_LIB_DEPS)
	$(CC) $(CFLAGS) -c -o $@  $<

$(OBJ_DIR)/ltest.o : $(LOGGER_TEST_DEPS)
	$(CC) $(CFLAGS) -c -o $@  $<

clean:
	rm -rf $(BUILD_DIR)/$(HOST)
