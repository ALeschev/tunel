#include <iostream>
#include <vector>
#include <atomic>
#include <memory>
#include <thread>
#include <algorithm>
#include "lock_free_mutex.hpp"
#include "lock_free_queue.hpp"

using namespace std;

lock_free_queue queue;
lock_free_mutex mutex;

void provider();
void consumer(int);

int main() {
  srand(time(NULL));

  thread *provider_threads = new thread[4];
  thread provider_thread(provider);
  for (auto i = 0; i < 4; ++i) {
    provider_threads[i] = thread(consumer, i + 1);
  }

  provider_thread.join();
  for (auto i = 0; i < 4; ++i) {
    provider_threads[i].join();
  }
  return 0;
}

void provider() {
  for (;;) {
    int count = (rand() % 10) + 1;
    vector <int> array;

    for (auto i = 0; i < count; ++i) {
      array.push_back(rand() % 100);
    }

    cout << "Initital: ";
    for (auto value : array) {
      cout << value << ' ';
    }
    cout << endl;

    queue.push(array);
  }
}

void consumer(int id) {
  for (;;) {
    shared_ptr <vector <int>> array = queue.pop();

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
