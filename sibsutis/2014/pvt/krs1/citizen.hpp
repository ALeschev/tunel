#pragma once
#include <iostream>

#include <iomanip>
#include <random>
#include <map>
#include <iostream>

using namespace std;

extern deque <ticket*> queue;
extern mutex queue_mutex;
extern int total_ticket;
extern int average_queue_len;

class citizen
{
private:
	int _activity;
	int tickets_success;
	int tickets_fail;
	bool active;
	int limit;
	default_random_engine gen;

public:
	void regen()
	{
		tickets_success = 0;
		tickets_fail = 0;
		limit = (rand() % 250) + 1;
		active = true;
		_activity = (rand() % 5) + 1;
	}

	citizen()
	{
		regen();
	}

	double activity()
	{
		exponential_distribution<double> dist(_activity);
		return dist(gen);
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
		// unique_lock <mutex> lock(queue_mutex);
		ticket *task = new ticket;
		future <bool> result = task->task.get_future();

		queue_mutex.lock();
		if (queue.size() < 10)
		{
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

		} else {
			queue_mutex.unlock();
		}
	}
};
