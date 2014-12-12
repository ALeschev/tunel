CFLAGS += -Wall -g
CFLAGS += -DDEBUG
#CFLAGS += -Werror
LIBS = -lpthread

CFLAGS += -DUSE_THREAD

all:
	gcc $(CFLAGS) work_crew.c -o work_crew $(LIBS)

clean:
	rm -f work_crew