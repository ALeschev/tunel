#pragma once

using namespace std;

extern deque <ticket*> queue;
extern mutex queue_mutex;

extern chrono::duration<double> averate_ticket_service;
extern chrono::duration<double> averate_ticket_wait;

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
		limit = (rand() % 100) + 1;
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

	void get_task()
	{
		chrono::time_point<std::chrono::system_clock> start, end;
		unique_lock <mutex> lock(queue_mutex);
		ticket *task;

		if (queue.empty())
		{
			return;
		}

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
		usleep(task->real_time * 100);
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
