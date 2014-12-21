#include <iostream>
#include <random>
#include <functional>
#include <thread>
#include <future>
#include <vector>
#include <deque>
#include <mutex>
#include <unistd.h>
#include "ticket.hpp"
#include "citizen.hpp"
#include "lawyer.hpp"

using namespace std;

void citizen_thread(int);
void lawyer_thread(int);
void winner_check_thread(int, int);

int main(void)
{
	thread *citizen_threads;
	thread *lawyer_threads;
	citizen *citizens;
	lawyer *lawyers;

	int deviation = 0;
	int citizen_count = 0;
	int lawyer_count = 0;

	srand(time(NULL));

	deviation = rand() % 1;
	citizen_count = (rand() % 20) + 1;
	lawyer_count = citizen_count deviation? +:- (rand() % 3);

	citizen_threads = new thread[citizen_count];
	lawyer_threads = new thread[lawyer_count];

	citizens = new citizen[citizen_count];
	lawyers = new lawyer[lawyer_count];

	for (int i = 0; i < citizen_count; ++i)
		citizen_threads[i] = thread(citizen_thread, &citizens[i]);

	for (int i = 0; i < lawyer_count; ++i)
		lawyer_threads[i] = thread(lawyer_thread, &lawyers[i]);

	thread winner(winner_check_thread, citizens, lawyers);
	winner.join();

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
			cout << "Citizen has left the city." << endl;
			inhabitant->leave();
			active = false;
		}

		sleep(10 - inhabitant->activity());
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
			cout << "Lawyer has left the city." << endl;
			official->leave();
			active = false;
		}
	}
}

void winner_check_thread(citizen *citizens, lawyer *lawyers)
{
	int citizen_count = sizeof (*citizens) / sizeof (citizen);
	int lawyer_count = sizeof (*lawyers) / sizeof (lawyer);
	int cur_citizen;
	int cur_lawyer;

	while (1)
	{
		cur_citizen = cur_lawyer = 0;

		for (int i = 0; i < citizen_count; ++i)
		{
			if (!citizens[i].status())
				continue;

			cur_citizen++;
		}
		if (!cur_citizen)
		{
			cout << "All citizens leave :(" << endl;
			exit(EXIT_SUCCESS);
		}

		for (int i = 0; i < lawyer_count; ++i)
		{
			if (lawyers[i].status())
				continue;

			cur_lawyer++;
		}
		if (!cur_lawyer)
		{
			cout << "All lawyer leave :(" << endl;
			exit(EXIT_SUCCESS);
		}

		usleep(500);
		cout << "usleep test" << endl;
	}
}
