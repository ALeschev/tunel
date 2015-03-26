#include "ClientSocket.h"
#include <iostream>
#include <string>

#include "ticket.hpp"
#include "citizen.hpp"
#include "lawyer.hpp"

using namespace std;

void citizen_thread(citizen *inhabitant);
void lawyer_thread(lawyer *official);
void winner_check_thread(citizen *citizens, lawyer *lawyers);

ClientSocket *r_socket;
ClientSocket *w_socket;

int main ( int argc, char* argv[] )
{
	thread *citizen_threads;
	thread *lawyer_threads;
	citizen *citizens;
	lawyer *lawyers;

	int citizen_count = 0;
	int lawyer_count = 0;

	srand(time(NULL));

	citizen_count = (rand() % 20) + 1;

	if (rand() % 1)
	{
		lawyer_count = citizen_count + (rand() % 3);
	} else {
		lawyer_count = citizen_count - (rand() % 3);
		if (lawyer_count < 1)
			lawyer_count = 1;
	}

	cout << "Emulation started: " << endl;
	cout << "Citizens: " << citizen_count << endl;
	cout << "Lawyers: " << lawyer_count << endl;

	citizen_threads = new thread[citizen_count];
	lawyer_threads = new thread[lawyer_count];

	citizens = new citizen[citizen_count];
	lawyers = new lawyer[lawyer_count];

	try
	{
		w_socket = new ClientSocket( "localhost", 30000 );
		r_socket = new ClientSocket( "localhost", 30001 );

		for (int i = 0; i < citizen_count; ++i)
			citizen_threads[i] = thread(citizen_thread, &citizens[i]);

		for (int i = 0; i < lawyer_count; ++i)
			lawyer_threads[i] = thread(lawyer_thread, &lawyers[i]);

		thread winner(winner_check_thread, citizens, lawyers);
		winner.join();

	} catch ( SocketException& e ) {
		std::cout << "Exception was caught:" << e.description() << "\n";
	}

	return 0;
}

void citizen_thread(citizen *inhabitant)
{
	bool active = true;

	while (active)
	{
		inhabitant->set_task(w_socket);
		if (inhabitant->limits_check())
		{
			cout << "Citizen has left the city." << endl;
			inhabitant->leave();
			active = false;
		}

		sleep(inhabitant->activity());
	}
}

void lawyer_thread(lawyer *official)
{
	bool active = true;

	while (active)
	{
		official->get_task(r_socket);
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
			if (!lawyers[i].status())
				continue;

			cur_lawyer++;
		}
		if (!cur_lawyer)
		{
			cout << "All lawyer leave :(" << endl;
			exit(EXIT_SUCCESS);
		}

		usleep(500000);
	}
}
