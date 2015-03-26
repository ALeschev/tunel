#pragma once

#include "ClientSocket.h"

using namespace std;

extern deque <ticket*> queue;
extern mutex queue_mutex;

class lawyer
{
private:
	int time;
	int tickets_fail;
	int tickets_success;
	bool active;
	int limit;

public:
	lawyer()
	{
		time = (rand() % 5) + 1;
		tickets_fail = 0;
		tickets_success = 0;
		active = true;
		limit = (rand() % 10) + 1;
	}

	bool limits_check()
	{
		return tickets_fail - tickets_success > limit;
	}

	void leave()
	{
		active = false;
	}

	bool status()
	{
		return active;
	}

	void get_task(ClientSocket *client_socket)
	{
		ticket *task = new ticket();

		*client_socket >> task;
		cout << "recv from 30001\n";

		task->promise_time = time;
		task->real_time = rand() % (time * 2) + 1;
		usleep(task->real_time);
		task->task.set_value(true);

		cout << "Lawyer: Real: " << task->real_time <<
		        " Promise " << task->promise_time << endl;

		if (task->real_time > task->promise_time)
		{
			tickets_fail++;
		} else {
			tickets_success++;
		}
	}
};
