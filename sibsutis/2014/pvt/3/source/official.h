#include <iostream>
#include <random>
#include <chrono>
#include <thread>

using namespace std;

class official
{
	public:
		official();

	private:
		float service_time;
};

official::official()
{
	random_device rd;
	mt19937 gen(rd());
	uniform_real_distribution<> dis(1, 5);

	service_time = dis(gen);
}
