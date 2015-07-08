CC = $(CROSS_COMPILER)gcc
STRIP = $(CROSS_COMPILER)strip
OBJ_DIR = build/$(ARCH)/obj
BIN_DIR = build/$(ARCH)/bin

SRC_DIR = ./source
HDR_DIR = ./source

INC = -I $(HDR_DIR)
CFLAGS = -g
LIBS = -lpthread -lrt

SERVER_TARGET = $(BIN_DIR)/chat_server

SERVER_OBJ += $(OBJ_DIR)/server.o \
              $(OBJ_DIR)/client_base.o \
              $(OBJ_DIR)/log.o \
              $(OBJ_DIR)/message.o \
              $(OBJ_DIR)/transport.o

#
# Dependence
#

LOG_DEPS         = $(SRC_DIR)/log.c \
                   $(HDR_DIR)/log.h

MESSAGE_DEPS     = $(SRC_DIR)/message.c \
                   $(HDR_DIR)/message.h

TRANSPORT_DEPS   = $(SRC_DIR)/transport.c \
                   $(HDR_DIR)/transport.h

CLIENT_BASE_DEPS = $(SRC_DIR)/client_base.c \
                   $(HDR_DIR)/client_base.h

SERVER_DEPS      = $(SRC_DIR)/server.c \
                   $(LOG_DEPS) \
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
	$(CC) $(CFLAGS) -o $@ $(SERVER_OBJ) $(LIBS)
	cp $(SERVER_TARGET) $(SERVER_TARGET)_uns
	$(STRIP) $(SERVER_TARGET)

#
# source files building
#

$(OBJ_DIR)/server.o : $(SERVER_DEPS) check_dir
	$(CC) $(CFLAGS) $(INC) -c -o $@  $<

$(OBJ_DIR)/client_base.o : $(CLIENT_BASE_DEPS) check_dir
	$(CC) $(CFLAGS) $(INC) -c -o $@  $<

$(OBJ_DIR)/log.o : $(LOG_DEPS) check_dir
	$(CC) $(CFLAGS) $(INC) -c -o $@  $<

$(OBJ_DIR)/message.o : $(MESSAGE_DEPS) check_dir
	$(CC) $(CFLAGS) $(INC) -c -o $@  $<

$(OBJ_DIR)/transport.o : $(TRANSPORT_DEPS) check_dir
	$(CC) $(CFLAGS) $(INC) -c -o $@  $<

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)