
LIB = libcli.a

CC	= $(CROSS_COMPILE)gcc
CXX	= $(CROSS_COMPILE)g++
AR	= $(CROSS_COMPILE)ar
STRIP 	= $(CROSS_COMPILE)strip

DEBUG = -g
OPTIM = -O2

CFLAGS  = -c -Wall $(DEBUG) $(OPTIM) -Wformat-security -Wno-write-strings -Wno-format-zero-length -I ../openssl/x86_64/usr/local/ssl/include
LDFLAGS += -shared -Wl,-soname,$(LIB).$(MAJOR).$(MINOR)
LIBPATH += -L. -L ../openssl/x86_64/usr/local/ssl/lib
LIBS = -lcrypt

CORE_PATH=./_core
INCLUDES  = -I$(CORE_PATH)
OBJS = $(HOST)/libcli.o

all: check-cc check-host check_ssl
	@mkdir -p $(HOST)
	@+make $(LIB)

check_ssl:
	if [ ! -d "./openssl/$(HOST)" ];           \
	then                                       \
	     cd ./openssl;                         \
	    ./configure.sh $(HOST);        \
	    ./make-and-install.sh $(HOST); \
	    cd -;                                  \
	fi

check-cc: 
	if test "$(CROSS_COMPILE)" = "" ; then \
	echo "CrossCompile not set"; \
	exit 1; \
	fi

check-host: 
	if test "$(HOST)" = "" ; then \
	echo "HOST not set"; \
	exit 2; \
	fi

$(HOST)/%.o: $(CORE_PATH)/%.c
	$(CXX) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(LIB): $(OBJS)
	ar -r $(HOST)/$(LIB) $(OBJS)

clean: check-cc check-host
	rm -f $(OBJS) $(HOST)/$(LIB)
