#include "ServerSocket.h"
#include <string>

#include <iostream>

bool active = false;

extern deque <ticket*> queue;
extern mutex queue_mutex;

void send_th ()
{
	ServerSocket w_server ( 30001 );
	ServerSocket w_sock;
	w_server.accept ( w_sock );

	cout << "Reader connacted" << endl;

	while (active)
	{
		// queue_mutex.lock();
		while (queue.size() > 0)
		{
			ticket *task;
			task = queue.front();
			queue.pop_front();

			try
			{
				w_sock << task;
				cout << "send to 30001\n";
			} catch (SocketException& e) {
				cout << e.description() << endl;
			}
		}
		// queue_mutex.unlock();

		// usleep(50000);
	}
}

int main ( int argc, char* argv[] )
{
	ticket *task = new ticket();
	thread send_thread;

	try
	{
		// Create the socket
		ServerSocket r_server ( 30000 );

		active = true;
		send_thread = thread(send_th);

		ServerSocket r_sock;
		r_server.accept ( r_sock );
		cout << "Writer connacted" << endl;

		try
		{
			while ( true )
			{
				r_sock >> task;
				cout << "recv from 30000\n";
				// new_sock << task;

				// queue_mutex.lock();
				if (queue.size() < 10)
				{
					queue.push_back(task);
				}
				// queue_mutex.unlock();
			}
		} catch ( SocketException& e) {
			cout << e.description() << endl;
		}
	} catch ( SocketException& e ) {
		std::cout << "Exception was caught:" << e.description() << "\nExiting.\n";
	}

	active = false;
	send_thread.join();

	return 0;
}