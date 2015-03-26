#include <iostream>
#include <vector>
#include <atomic>
#include <memory>
#include <thread>
#include <algorithm>
#include "lock_free_queue.hpp"

using namespace std;

lock_free_queue queue;
lock_free_queue* queues;

unsigned int threads_count = thread::hardware_concurrency();
void producer();
void consumer(int);
void main_consumer();
thread *consumer_threads = new thread[threads_count];

int main() {
  srand(time(NULL));

  queues = new lock_free_queue[threads_count];
  thread producer_thread(producer);
  thread consumer_thread(main_consumer);
  for (auto i = 0; i < 4; ++i) {
    consumer_threads[i] = thread(consumer, i + 1);
  }

  producer_thread.join();
  return 0;
}

void producer() {
  for (;;) {
    int count = (rand() % 1000) + 1;
    vector <int> array;

    for (auto i = 0; i < count; ++i) {
      array.push_back(rand() % count);
    }

    cout << "Initital: ";
    for (auto value : array) {
      cout << value << ' ';
    }
    cout << endl;

    queue.push(array);
  }
}

void main_consumer() {
  for (;;) {
    for (auto i = 0; i < threads_count; i++) {
      shared_ptr <vector <int>> array = queue.pop();

      if (array != nullptr) {
        queues[i].push(*array);
      }
    }
  }
}

void consumer(int id) {
  for (;;) {
    shared_ptr <vector <int>> array = queues[id].pop();

    if (array != nullptr) {
      sort(array->begin(), array->end());

      cout << "Sorted (" << id << "): ";
      for (auto value : *array) {
        cout << value << ' ';
      }
      cout << endl;
    }
  }
}
