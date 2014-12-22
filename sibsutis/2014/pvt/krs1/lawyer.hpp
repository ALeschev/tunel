#pragma once

#include <algorithm>

using namespace std;

extern deque <ticket*> queue;
extern mutex queue_mutex;

extern chrono::duration<double> averate_ticket_service;
extern chrono::duration<double> averate_ticket_wait;

class lawyer
{
private:
	int tickets_fail;
	int tickets_success;
	bool active;
	int limit;

public:
	void regen()
	{
		tickets_fail = 0;
		tickets_success = 0;
		active = true;
		limit = (rand() % 250) + 1;
	}

	lawyer()
	{
		regen();
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

	void shake()
	{
		deque <ticket*> foo;
		unique_lock <mutex> lock(queue_mutex);

		if (queue.empty())
			return;

		queue.swap(foo);
	}

	void get_task()
	{
		chrono::time_point<std::chrono::system_clock> start, end;
		unique_lock <mutex> lock(queue_mutex);
		int time = (rand() % 5) + 1;
		ticket *task;

		if (queue.empty())
			return;

		std::sort(queue.begin(), queue.end());

		start = std::chrono::system_clock::now();

		task = queue.front();
		queue.pop_front();
		lock.unlock();

		end = std::chrono::system_clock::now();

		statistics_mutex.lock();
		averate_ticket_wait += end - start;
		statistics_mutex.unlock();

		task->promise_time = time;
		task->real_time = rand() % (time * 2) + 1;
		usleep(task->real_time);
		task->task.set_value(true);

		// cout << "Lawyer: Real: " << task->real_time <<
		//         " Promise " << task->promise_time << endl;

		end = std::chrono::system_clock::now();

		statistics_mutex.lock();
		averate_ticket_service += end - start;
		statistics_mutex.unlock();

		if (task->real_time > task->promise_time)
		{
			tickets_fail++;
		} else {
			tickets_success++;
		}
	}
};
