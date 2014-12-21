#pragma once

using namespace std;

extern deque <ticket*> queue;
extern mutex queue_mutex;

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

	void set_task()
	{
		ticket *task = new ticket;
		future <bool> result = task->task.get_future();

		queue_mutex.lock();
		if (queue.size() < 10)
		{
			queue.push_back(task);
			queue_mutex.unlock();

			result.get();
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
