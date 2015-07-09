CC = $(CROSS_COMPILER)gcc
STRIP = $(CROSS_COMPILER)strip
OBJ_DIR = build/$(ARCH)/obj
BIN_DIR = build/$(ARCH)/bin

HOST = x86_64

# ifeq ($(CROSS_COMPILE),x86_64-linux-gnu-)
# HOST=x86_64-linux-gnu
# endif

SERVER_SRC_DIR = ./server
SERVER_HDR_DIR = ./server

TRANSPORT_SRC_DIR = ./transport/include
TRANSPORT_HDR_DIR = ./transport/src
TRANSPORT_LIB_DIR = ./transport/.build/$(HOST)/.pbyte
TRANSPORT_ZMQ_LIB =./transport/.build/$(HOST)/.zmq/usr/local/lib

LOGGER_SRC_DIR = ./logger
LOGGER_HDR_DIR = ./logger

CFLAGS  = -g
INC     = -I $(SERVER_HDR_DIR) -I $(TRANSPORT_SRC_DIR) -I $(LOGGER_HDR_DIR)
LDFLAGS = -L $(TRANSPORT_LIB_DIR) -L $(TRANSPORT_ZMQ_LIB)
LIBS    = -lpthread -lrt -lpbyte -lzmq

SERVER_TARGET = $(BIN_DIR)/chat_server

SERVER_OBJ += $(OBJ_DIR)/server.o \
              $(OBJ_DIR)/client_base.o \
              $(OBJ_DIR)/logger.o \
              $(OBJ_DIR)/message.o \
              $(OBJ_DIR)/transport.o

#
# Dependence
#

LOGGER_DEPS      = $(LOGGER_SRC_DIR)/logger.c \
                   $(LOGGER_HDR_DIR)/logger.h

MESSAGE_DEPS     = $(SERVER_SRC_DIR)/message.c \
                   $(SERVER_HDR_DIR)/message.h

TRANSPORT_DEPS   = $(SERVER_SRC_DIR)/transport.c \
                   $(SERVER_HDR_DIR)/transport.h

CLIENT_BASE_DEPS = $(SERVER_SRC_DIR)/client_base.c \
                   $(SERVER_HDR_DIR)/client_base.h

SERVER_DEPS      = $(SERVER_SRC_DIR)/server.c \
                   $(LOGGER_DEPS) \
                   $(MESSAGE_DEPS) \
                   $(TRANSPORT_DEPS) \
                   $(CLIENT_BASE_DEPS)

all: check_dir $(SERVER_TARGET)

check_dir:
	mkdir -p $(OBJ_DIR)
	mkdir -p $(BIN_DIR)

#
# Targets building
#

$(SERVER_TARGET): $(SERVER_OBJ) check_dir
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(SERVER_OBJ) $(LIBS)
	cp $(SERVER_TARGET) $(SERVER_TARGET)_uns
	$(STRIP) $(SERVER_TARGET)

#
# source files building
#

$(OBJ_DIR)/server.o : $(SERVER_DEPS) check_dir
	$(CC) $(CFLAGS) $(INC) -c -o $@  $<

$(OBJ_DIR)/client_base.o : $(CLIENT_BASE_DEPS) check_dir
	$(CC) $(CFLAGS) $(INC) -c -o $@  $<

$(OBJ_DIR)/logger.o : $(LOGGER_DEPS) check_dir
	$(CC) $(CFLAGS) $(INC) -c -o $@  $<

$(OBJ_DIR)/message.o : $(MESSAGE_DEPS) check_dir
	$(CC) $(CFLAGS) $(INC) -c -o $@  $<

$(OBJ_DIR)/transport.o : $(TRANSPORT_DEPS) check_dir
	$(CC) $(CFLAGS) $(INC) -c -o $@  $<

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)