#pragma once
#include <iostream>

using namespace std;

extern deque <ticket*> queue;
extern mutex queue_mutex;
extern int total_ticket;
extern int average_queue_len;
// extern chrono::duration<double> averate_ticket_service;
// extern chrono::duration<double> averate_ticket_wait;

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
		active = true;
		limit = (rand() % 5) + 1;
	}

	int generate()
	{
		default_random_engine gen;
		exponential_distribution<double> dist(1);
		_activity = dist(gen);
	}

	int activity()
	{
		generate();
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

	void set_task()
	{
		// chrono::time_point<std::chrono::system_clock> start, end;
		ticket *task = new ticket;
		future <bool> result = task->task.get_future();

		queue_mutex.lock();
		if (queue.size() < 10)
		{
			// start = std::chrono::system_clock::now();
			task->priority = (rand() % 5) + 1;
			queue.push_back(task);
			total_ticket++;
			average_queue_len += queue.size();
			queue_mutex.unlock();

			result.get();

			// cout << "Citizen: Real: " << task->real_time <<
			//         " Promise " << task->promise_time << endl;

			if (task->real_time > task->promise_time)
			{
				tickets_fail++;
			} else {
				tickets_success++;
			}

			// averate_ticket_service = start - end;
		} else {
			queue_mutex.unlock();
		}
	}
};
