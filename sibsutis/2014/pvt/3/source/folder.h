#include <iostream>
#include <random>
#include <chrono>
#include <thread>
#include <set>
#include <mutex>

using namespace std;

class folder
{
	public:
		folder(int size);
		int set_request(int req_num);
		int get_request(int req_num);

	private:
		unsigned int size;
		set<int> requests;
		mutex mtx;
};

folder::folder(int size)
{
	this->size = size;
}

int folder::set_request(int req_num)
{
	mtx.lock();

	if (requests.size() >= size)
	{
		mtx.unlock();
		return -1;
	}

	requests.insert(req_num);

	mtx.unlock();

	return 0;
}

int folder::get_request(int req_num)
{
	int req;

	mtx.lock();

	if (!requests.empty())
	{
		mtx.unlock();
		return 0;
	}

	req = *requests.begin();

	mtx.unlock();

	return req;
}