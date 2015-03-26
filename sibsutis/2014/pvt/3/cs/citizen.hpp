#pragma once
#include <iostream>

#include "ClientSocket.h"
#include <chrono>
#include <ctime>

using namespace std;

// extern deque <ticket*> queue;
// extern mutex queue_mutex;

class citizen
{
private:
	int _activity;
	int tickets_success;
	int tickets_fail;
	bool active;
	int limit;

public:
	citizen()
	{
		tickets_success = 0;
		tickets_fail = 0;
		_activity = (rand() % 5) + 1;
		active = true;
		limit = (rand() % 10) + 1;
	}

	int activity()
	{
		return _activity;
	}

	bool limits_check()
	{
		return tickets_success - tickets_fail > limit;
	}

	void leave()
	{
		active = false;
	}

	bool status()
	{
		return active;
	}

	void set_task(ClientSocket *client_socket)
	{
		ticket *task = new ticket();
		future <bool> result = task->task.get_future();

		*client_socket << task;
		cout << "send to 30000\n";

		result.get();

		cout << "Sitizen: Real: " << task->real_time <<
		        " Promise " << task->promise_time << endl;

		if (task->real_time > task->promise_time)
		{
			tickets_fail++;
		} else {
			tickets_success++;
		}
	}
};
