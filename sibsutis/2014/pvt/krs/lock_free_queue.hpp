#pragma once

using namespace std;

extern lock_free_mutex mutex;

class lock_free_queue
{
private:
	struct node
	{
		shared_ptr <vector <int>> data;
		node* next;
		node():next(nullptr){};
	};

	atomic<node*> head;
	atomic<node*> tail;

	node* pop_head()
	{
		mutex.lock();
		node* const old_head = head.load();

		if (old_head == tail.load())
		{
			mutex.unlock();
			return nullptr;
		}

		head.store(old_head->next);
		mutex.unlock();

		return old_head;
	}

public:
	lock_free_queue(): head(new node), tail(head.load()) {}
	lock_free_queue(const lock_free_queue& other) = delete;
	lock_free_queue& operator = (const lock_free_queue& other) = delete;

	~lock_free_queue()
	{
		while(node* const old_head = head.load())
		{
			head.store(old_head->next);
			delete old_head;
		}
	}

	shared_ptr <vector <int>> pop()
	{
		node* old_head = pop_head();

		if (!old_head)
			return shared_ptr <vector <int>>();

		shared_ptr <vector <int>> const res(old_head->data);
		delete old_head;

		return res;
	}

	void push(vector<int> new_value)
	{
		shared_ptr <vector <int>> new_data(make_shared <vector <int>> (new_value));

		node* p = new node;
		node* const old_tail = tail.load();

		old_tail->data.swap(new_data);
		old_tail->next = p;
		tail.store(p);
	}
};
