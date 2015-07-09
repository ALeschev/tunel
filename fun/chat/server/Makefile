CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
STRIP = $(CROSS_COMPILE)strip
PROJ_DIR = ..
OBJ_DIR   = $(PROJ_DIR)/$(build_dir)/obj
BIN_DIR   = $(PROJ_DIR)/$(build_dir)/bin

HOST = x86_64

# ifeq ($(CROSS_COMPILE),x86_64-linux-gnu-)
# HOST=x86_64-linux-gnu
# endif

SERVER_SRC_DIR = ./
SERVER_HDR_DIR = ./

TRANSPORT_SRC_DIR = $(PROJ_DIR)/transport/include
TRANSPORT_HDR_DIR = $(PROJ_DIR)/transport/src
TRANSPORT_LIB_DIR = $(PROJ_DIR)/transport/.build/$(HOST)/.pbyte
TRANSPORT_ZMQ_LIB =$(PROJ_DIR)/transport/.build/$(HOST)/usr/local/lib

LOGGER_SRC_DIR = $(PROJ_DIR)/logger
LOGGER_HDR_DIR = $(PROJ_DIR)/logger

CFLAGS  = -g -Wno-write-strings
INC     = -I $(SERVER_HDR_DIR) -I $(TRANSPORT_SRC_DIR) -I $(LOGGER_HDR_DIR)
LDFLAGS = -L $(TRANSPORT_LIB_DIR) -L $(TRANSPORT_ZMQ_LIB)
LIBS    = -lpthread -lpbyte -lzmq -lpthread

SERVER_TARGET = $(BIN_DIR)/chat_server

SERVER_OBJ += $(OBJ_DIR)/server.o \
              $(OBJ_DIR)/client_base.o \
              $(OBJ_DIR)/logger.o

#
# Dependence
#

LOGGER_DEPS      = $(LOGGER_SRC_DIR)/logger.c \
                   $(LOGGER_HDR_DIR)/logger.h

CLIENT_BASE_DEPS = $(SERVER_SRC_DIR)/client_base.c \
                   $(SERVER_HDR_DIR)/client_base.h

SERVER_DEPS      = $(SERVER_SRC_DIR)/server.c \
                   $(LOGGER_DEPS) \
                   $(CLIENT_BASE_DEPS)

all: check_dir $(SERVER_TARGET)

check_dir:
	mkdir -p $(OBJ_DIR)
	mkdir -p $(BIN_DIR)

#
# Targets building
#

$(SERVER_TARGET): $(SERVER_OBJ) check_dir
	$(CXX) $(CFLAGS) $(LDFLAGS) -o $@ $(SERVER_OBJ) $(LIBS)
	cp $(SERVER_TARGET) $(SERVER_TARGET)_uns
	$(STRIP) $(SERVER_TARGET)

#
# source files building
#

$(OBJ_DIR)/server.o : $(SERVER_DEPS) check_dir
	$(CXX) $(CFLAGS) $(INC) -c -o $@  $<

$(OBJ_DIR)/client_base.o : $(CLIENT_BASE_DEPS) check_dir
	$(CXX) $(CFLAGS) $(INC) -c -o $@  $<

$(OBJ_DIR)/logger.o : $(LOGGER_DEPS) check_dir
	$(CXX) $(CFLAGS) $(INC) -c -o $@  $<

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)