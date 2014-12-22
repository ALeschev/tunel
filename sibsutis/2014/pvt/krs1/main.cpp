#include <iostream>
#include <random>
#include <functional>
#include <thread>
#include <future>
#include <vector>
#include <deque>
#include <mutex>
#include <iomanip>
#include <unistd.h>
#include "ticket.hpp"
#include "citizen.hpp"
#include "lawyer.hpp"

using namespace std;

extern int total_ticket;
extern int average_queue_len;
extern chrono::duration<double> averate_ticket_service;
extern chrono::duration<double> averate_ticket_wait;

void lawyer_thread_shaker(lawyer *shaker);
void citizen_thread(citizen *inhabitant);
void lawyer_thread(lawyer *official);
void winner_check_thread(citizen *citizens, int citizen_count,
                         lawyer *lawyers, int lawyer_count);

thread *citizen_threads;
thread *lawyer_threads;

int main(void)
{
	citizen *citizens;
	lawyer *lawyers;

	int citizen_count = 0;
	int lawyer_count = 0;

	srand(time(NULL));

	citizen_count = (rand() % 10) + 1;

	if (rand() % 1)
		lawyer_count = citizen_count + (rand() % 3);
	else
		lawyer_count = citizen_count - (rand() % 4) + 1;


	cout << "Emulation started: " << endl;
	cout << "Citizens: " << citizen_count << endl;
	cout << "Lawyers: " << lawyer_count << endl;

	citizen_threads = new thread[citizen_count];
	lawyer_threads = new thread[lawyer_count];

	citizens = new citizen[citizen_count];
	lawyers = new lawyer[lawyer_count];

	for (int i = 0; i < citizen_count; ++i)
		citizen_threads[i] = thread(citizen_thread, &citizens[i]);

	for (int i = 0; i < lawyer_count; ++i)
		lawyer_threads[i] = thread(lawyer_thread, &lawyers[i]);

	lawyer lawy_shaker;
	thread shaker(lawyer_thread_shaker, &lawy_shaker);

	thread winner(winner_check_thread, citizens, citizen_count,
	                                   lawyers, lawyer_count);
	winner.join();
	shaker.join();

	return 0;
}

void citizen_thread(citizen *inhabitant)
{
	bool active = true;

	while (active)
	{
		inhabitant->set_task();
		if (inhabitant->limits_check())
		{
			cout << "Citizen left." << endl;
			inhabitant->leave();
			active = false;
		} else {
			usleep(inhabitant->activity() * 10000);
		}
	}
}

void lawyer_thread(lawyer *official)
{
	bool active = true;

	while (active)
	{
		official->get_task();
		if (official->limits_check())
		{
			cout << "Lawyer left." << endl;
			official->leave();
			active = false;
		}
	}
}

void lawyer_thread_shaker(lawyer *shaker)
{
	bool active = true;

	while (active)
	{
		sleep(5);
		shaker->shake();
		cout << "Shake!\n";
	}
}

void winner_check_thread(citizen *citizens, int citizen_count,
                         lawyer *lawyers, int lawyer_count)
{
	int cur_citizen;
	int cur_lawyer;

	while (1)
	{
		cur_citizen = cur_lawyer = 0;

		for (int i = 0; i < citizen_count; ++i)
		{
			if (!citizens[i].status())
			{
				if (rand()%100 == 0)
				{
					citizen_threads[i].join();
					cout << "New Citizen spawned" << endl;
					citizen_threads[i] = thread(citizen_thread, &citizens[i]);
				} else {
					continue;
				}
			}

			cur_citizen++;
		}
		if (!cur_citizen)
		{
			cout << "All citizens leave :(" << endl;
			break;
		}

		for (int i = 0; i < lawyer_count; ++i)
		{
			if (!lawyers[i].status())
			{
				if (rand()%100 == 0)
				{
					lawyer_threads[i].join();
					cout << "New Lawyer spawned" << endl;
					lawyer_threads[i] = thread(lawyer_thread, &lawyers[i]);
				} else {
					continue;
				}
			}

			cur_lawyer++;
		}
		if (!cur_lawyer)
		{
			cout << "All lawyer leave :(" << endl;
			break;
		}

		usleep(500000);


	}

	cout.precision(3);

	cout << "total ticket: " << total_ticket << endl;
	cout << "average queue len: " << average_queue_len / total_ticket << endl;
	cout << "average service: " << averate_ticket_service.count() / total_ticket << "s\n";
	cout << "average wait: " << averate_ticket_wait.count() / total_ticket<< "s\n";

	exit(EXIT_SUCCESS);
}
