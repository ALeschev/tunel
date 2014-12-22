CFLAGS += -Wall -g
CFLAGS += -DDEBUG
#CFLAGS += -Werror
LIBS = -Wall -pedantic -pthread -std=c++14 -O2

CFLAGS += -DUSE_THREAD

all:
	g++ $(CFLAGS) work_crew.cpp -o work_crew $(LIBS)

clean:
	rm -f work_crew