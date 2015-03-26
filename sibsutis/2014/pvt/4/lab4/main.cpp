/****************************************/
/*              pvt14 lab4              */
/****************************************/

#include "lib_lab4.hpp"

using namespace std;

int main()
{
	thread th_p[NUM_THREAD_PROD];
	thread th_c[NUM_THREAD_CONS];

	srand(time(NULL));

	for (auto i = 0; i < NUM_THREAD_PROD; i++) {
		th_p[i] = thread(producer);
	}

	for (auto i = 0; i < NUM_THREAD_CONS; i++) {
		th_c[i] = thread(consumer, i);
	}

	for (auto i = 0; i < NUM_THREAD_PROD; i++) {
		th_p[i].join();
	}

	for (auto i = 0; i < NUM_THREAD_CONS; i++) {
		th_c[i].join();
	}

	return 0;
}