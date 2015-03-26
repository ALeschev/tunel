all: clean simple_server simple_client

simple_server:
	g++ -g simple_server_main.cpp -o simple_server -std=c++11 -pthread -pedantic -O2

simple_client:
	g++ -g simple_client_main.cpp -o simple_client -std=c++11 -pthread -pedantic -O2

clean:
	rm -f *.o simple_server simple_client