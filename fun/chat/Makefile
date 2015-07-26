CLIENT_DIR    = ./client
SERVER_DIR    = ./server
TRANSPORT_DIR = ./transport
CONSOLE_DIR   = ./libcli

HOST = x86_64
CROSS_COMPILE = x86_64-linux-gnu-
PLATFORM = x86_64-linux-gnu

ifeq ($(host),mr3020)
HOST=mr3020
CROSS_COMPILE=mips-openwrt-linux-
PLATFORM=mips-openwrt-linux
endif

BUILD_DIR = .build
TARGET_DIR = $(BUILD_DIR)/$(HOST)

# ifeq ($(CROSS_COMPILE),x86_64-linux-gnu-)
# HOST=x86_64-linux-gnu
# endif

all: check_dir slibcli stransport sclient sserver

check_dir:
	mkdir -p $(BUILD_DIR)

sclient: check_dir
	make -C $(CLIENT_DIR) build_dir=$(TARGET_DIR) CROSS_COMPILE=$(CROSS_COMPILE) HOST=$(HOST)

sserver: check_dir
	make -C $(SERVER_DIR) build_dir=$(TARGET_DIR) CROSS_COMPILE=$(CROSS_COMPILE) HOST=$(HOST)

stransport: check_dir
	#make -C $(TRANSPORT_DIR) platform=$(PLATFORM) CROSS_COMPILE=$(CROSS_COMPILE) HOST=$(HOST) build_zmq
	make -C $(TRANSPORT_DIR) platform=$(PLATFORM) CROSS_COMPILE=$(CROSS_COMPILE) HOST=$(HOST)

slibcli: check_dir
	make -C $(CONSOLE_DIR) platform=$(PLATFORM) CROSS_COMPILE=$(CROSS_COMPILE) HOST=$(HOST)

clean:
	rm -rf $(BUILD_DIR)
	make -C $(CLIENT_DIR) CROSS_COMPILE=$(CROSS_COMPILE) clean
	make -C $(SERVER_DIR) CROSS_COMPILE=$(CROSS_COMPILE) clean
	make -C $(TRANSPORT_DIR) platform=$(PLATFORM) CROSS_COMPILE=$(CROSS_COMPILE) clean

distclean: clean
	make -C $(TRANSPORT_DIR) platform=$(PLATFORM) CROSS_COMPILE=$(CROSS_COMPILE) distclean

