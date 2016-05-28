BIN_NAME = lz78
OBJ_DIR  =./obj
SRC_DIR  =./source

OBJ_DEP  = $(OBJ_DIR)/lz78.o $(OBJ_DIR)/lz78_pack_unpack.o \
           $(OBJ_DIR)/lz78_dictionary.o

CFLAGS= -Wall -g -O0

.PHONY: check_dir

all: check_dir $(BIN_NAME)

clean:
	@rm -rf $(OBJ_DIR) $(BIN_NAME)

check_dir:
	@mkdir -p $(OBJ_DIR)

$(BIN_NAME): $(OBJ_DEP)
	gcc $(CFLAGS) -o $@ $(OBJ_DEP)

$(OBJ_DIR)/lz78.o: $(SRC_DIR)/lz78.c $(SRC_DIR)/lz78.h \
                   $(SRC_DIR)/lz78_pack_unpack.h
	gcc $(CFLAGS) -c -o $@  $<

$(OBJ_DIR)/lz78_pack_unpack.o: $(SRC_DIR)/lz78_pack_unpack.c $(SRC_DIR)/lz78_pack_unpack.h \
                        $(SRC_DIR)/lz78.h $(SRC_DIR)/lz78_dictionary.c \
                        $(SRC_DIR)/lz78_dictionary.h
	gcc $(CFLAGS) -c -o $@  $<

$(OBJ_DIR)/lz78_dictionary.o: $(SRC_DIR)/lz78_dictionary.c $(SRC_DIR)/lz78_dictionary.h
	gcc $(CFLAGS) -c -o $@  $<
