#ifndef __TICKET_HPP__
#define __TICKET_HPP__

#include <future>
#include <deque>
#include <mutex>
#include <functional>
#include <thread>

using namespace std;

class ticket
{
public:
	promise <bool> task;
	int promise_time;
	int real_time;

	ticket() {}
};

static int total_ticket = 0;
static mutex queue_mutex;
static deque <ticket*> queue;

#endif /*__TICKET_HPP__*/