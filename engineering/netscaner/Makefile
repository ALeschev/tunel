CC = $(CROSS_COMPILER)gcc
STRIP = $(CROSS_COMPILER)strip
OBJ_DIR = build/$(ARCH)/obj
BIN_DIR = build/$(ARCH)/bin

SRC_DIR = .
HDR_DIR = .

NETSCANER_TARGET = $(BIN_DIR)/netscaner

NETSCANER_OBJ += $(OBJ_DIR)/netscaner.o \
                 $(OBJ_DIR)/icmp_core.o

#
# Dependence
#

ICMP_CORE_DEPS = $(SRC_DIR)/icmp_core.c \
                 # $(HDR_DIR)/icmp_core.h \

NETSCANER_DEPS = $(SRC_DIR)/netscaner.c \
                 # $(HDR_DIR)/netscaner.h \
                 $(ICMP_CORE_DEPS)



CFLAGS = -g

LIBS = -lpthread

INC = -I ./

all: check_dir $(NETSCANER_TARGET)

check_dir:
	mkdir -p $(OBJ_DIR)
	mkdir -p $(BIN_DIR)

#
# Targets building
#

$(NETSCANER_TARGET): $(NETSCANER_OBJ)
	$(CC) $(CFLAGS) -o $@ $(NETSCANER_OBJ) $(LIBS)
	cp $(NETSCANER_TARGET) $(NETSCANER_TARGET)_uns
	$(STRIP) $(NETSCANER_TARGET)

#
# source files building
#

$(OBJ_DIR)/netscaner.o : $(NETSCANER_DEPS)
	$(CC) $(CFLAGS) $(INC) -c -o $@  $<

$(OBJ_DIR)/icmp_core.o : $(ICMP_CORE_DEPS)
	$(CC) $(CFLAGS) $(INC) -c -o $@  $<

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)