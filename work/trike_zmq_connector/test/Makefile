PREFIX=TRIKE_TEST_
TOP_DIR=../
FILES:=$(patsubst %.cc,$(PREFIX)%.o,$(wildcard *.cc))

OS_NAME = $(strip $(shell uname | tr '[A-Z]' '[a-z]'))

ifeq ($(OS_NAME),darwin)
OS_LIBS=-pthread
else ifeq($(OS_NAME),linux)
OS_LIBS=-lpthread -lrt
endif

TEST_CFLAGS=-I $(TOP_DIR)lib/gtest-1.7.0/include -I $(TOP_DIR)src/ -I$(TOP_DIR)include/
TEST_LDFLAGS=$(TOP_DIR)lib/gtest-1.7.0/lib/.libs/libgtest.a $(TOP_DIR)lib/libzmq.a $(TOP_DIR)lib/libprotobuf-c.a
TEST_LDFLAGS+=$(OS_LIBS)
ALL_OBJECTS:=$(patsubst %.o,%.o,$(wildcard $(TOP_DIR)obj/*.o))

all: compile

compile: $(FILES)

$(PREFIX)%unit_test.o:
	$(CC) $*unit_test.cc $(TEST_CFLAGS) $(TEST_LDFLAGS) -o $*unit_test.out
	./$*unit_test.out

$(PREFIX)%_integration_test.o:
	$(CC) $(TEST_CFLAGS) -c $*_integration_test.cc -o $(TOP_DIR)obj/$*_integration_test.o
	$(CC) $(ALL_OBJECTS) $(TOP_DIR)obj/$*_integration_test.o $(TEST_LDFLAGS) -o $*_integration_test.out
	./$*_integration_test.out

clean:
	rm -f *.out
