/****************************************/
/*		    				threads								*/
/****************************************/

#include "lib_lab4.hpp"

using namespace std;

QueueArray queue[NUM_THREAD_CONS];
queueArrayBaseType base;

bool sort_v(int i, int j) { return i < j; }

void producer()
{
	vector<int> v;
	bool status;
	int num_q = 0;

	for (;;) {
		int rand_len = rand() % (MAX_ELEM_RAND - MIN_ELEM_RAND + 1) + MIN_ELEM_RAND;
		for (auto i = 0; i < rand_len; i++) {
			v.push_back(rand() % (MAX_ELEM_RAND - MIN_ELEM_RAND + 1) + MIN_ELEM_RAND);
		}

		status = false;
		while (!status) {
			for (auto i = num_q; i < NUM_THREAD_CONS; i++) {
				cout << "queue [" << num_q << "] = ";
				for (int j : v)
				cout << j << " ";
				cout << endl;
				if (isFullQueueArray(&queue[i]) == 0) {
					putQueueArray(&queue[i], v);
					status = true;
					num_q = ++i;
					num_q = num_q == NUM_THREAD_CONS ? 0 : num_q;
					break;
				}
			}
		}
		v.clear();
		//sleep(1);
	}
}

void consumer(int id)
{
	while(1) {
		if (isEmptyQueueArray(&queue[id]) == 0) {
			getQueueArray(&queue[id], &base);
			cout << id << " -- я взял" << endl;
			sort(base.begin(), base.end(), sort_v);
			for (int i : base)
			    cout << i << " ";
			cout << endl;
    			base.clear();
		}
	}
}
