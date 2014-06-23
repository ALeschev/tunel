SOURCE_DIR=./
INCLUDE_DIR=$(SOURCE_DIR)include/
OBJ_DIR=../obj/

CFLAGS=-I $(INCLUDE_DIR)

all: obj cp_obj

obj: $(patsubst %.c,%.o,$(wildcard *.c))

%.o: %.c
	$(CC) -c $(CFLAGS) $<

cp_obj:
	mv *.o $(OBJ_DIR)

clean:
	rm -f $(SOURCE_DIR)*.c
	rm -f $(INCLUDE_DIR)*.h
	rm -f $(SOURCE_DIR)*.sample