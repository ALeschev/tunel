CFLAGS += -Wall -g
CFLAGS += -DDEBUG
#CFLAGS += -Werror
LIBS = -lpthread

all:
	gcc $(CFLAGS) work_crew.c -o work_crew $(LIBS)

clean:
	rm -f work_crew
	rm -rf ./dir1/*_e ./dir1/*.out
