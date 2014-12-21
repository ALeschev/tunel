#pragma once

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