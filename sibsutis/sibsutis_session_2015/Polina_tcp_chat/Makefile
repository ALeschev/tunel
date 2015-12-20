CFLAGS = -Wall -pedantic

all: build_client build_server

build_client: client.c logger.c logger.h
	gcc $(CFLAGS) client.c logger.c -o client

build_server: server.c logger.c logger.h
	gcc $(CFLAGS) server.c logger.c -o server

clean:
	rm -f server client