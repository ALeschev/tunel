#include <iostream>
#include <random>
#include <chrono>
#include <thread>

#include "folder.h"

using namespace std;

class inhabitant
{
	public:
		inhabitant();
		void *write_request(void *);

	private:
		float activity;
};

inhabitant::inhabitant()
{
	random_device rd;
	mt19937 gen(rd());
	uniform_real_distribution<> dis(1, 5);

	activity = dis(gen);
}

void *inhabitant::write_request(void *arg)
{
	int seed = chrono::system_clock::now().time_since_epoch().count();
	default_random_engine generator (seed);

	exponential_distribution<double> distribution (activity);

	for (int i = 0; i < 10; i++)
	{
		double number = distribution(generator);
		chrono::duration<double> period ( number );
		this_thread::sleep_for( period );
		cout << "beep!" << endl;
	}
}

