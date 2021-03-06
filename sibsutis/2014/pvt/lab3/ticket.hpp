#pragma once

#include <chrono>
#include <ctime>

using namespace std;

class ticket
{
public:
	promise <bool> task;
	int promise_time;
	int real_time;

	ticket() {}
};

mutex queue_mutex;
deque <ticket*> queue;

mutex statistics_mutex;
std::chrono::duration<double> averate_ticket_service;
std::chrono::duration<double> averate_ticket_wait;
int total_ticket = 0;
int average_queue_len = 0;