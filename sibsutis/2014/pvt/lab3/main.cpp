#include <iostream>
#include <random>
#include <functional>
#include <thread>
#include <future>
#include <vector>
#include <deque>
#include <mutex>
#include <unistd.h>
#include "ticket.hpp"
#include "citizen.hpp"
#include "lawer.hpp"

using namespace std;

mutex queue_mutex;
deque <ticket*> queue;
citizen *citizens;
lawer *lawers;
int limit;

void citizen_thread(int);
void lawer_thread(int);
void winner_check_thread(int, int);

int main() {
  srand(time(NULL));

  int citizen_count = (rand() % 20) + 1;
  int lawer_count = citizen_count + (rand() % 3);
  thread *citizen_threads = new thread[citizen_count];
  thread *lawer_threads = new thread[lawer_count];
  limit = 2;
//  limit = (rand() % 5) + 1;

  citizens = new citizen[citizen_count];
  lawers = new lawer[lawer_count];

  for (int i = 0; i < citizen_count; ++i) {
    citizen_threads[i] = thread(citizen_thread, i);
  }
  for (int i = 0; i < lawer_count; ++i) {
    lawer_threads[i] = thread(lawer_thread, i);
  }

  thread winner(winner_check_thread, citizen_count, lawer_count);
  winner.join();

  return 0;
}

void citizen_thread(int id) {
  while (1) {
    citizens[id].set_task();
    if (citizens[id].time_to_leave(limit)) {
      cout << "Citizen " << id << " has left the city." << endl;
      citizens[id].leave();
      break;
    }

    sleep(10 - citizens[id].activity());
  }
}

void lawer_thread(int id) {
  while (1) {
    lawers[id].get_task();
    if (lawers[id].time_to_leave(limit)) {
      cout << "Lawer " << id << " has left the city." << endl;
      lawers[id].leave();
      break;
    }
  }
}

void winner_check_thread(int citizen_count, int lawer_count) {
  bool active = false;
  while (1) {
    active = false;
    for (int i = 0; i < citizen_count; ++i) {
      if (citizens[i].status()) {
        active = true;
        break;
      }
    }
    if (!active) {
      cout << "Lawers win" << endl;
      exit(EXIT_SUCCESS);
    }

    active = false;
    for (int i = 0; i < lawer_count; ++i) {
      if (lawers[i].status()) {
        active = true;
        break;
      }
    }
    if (!active) {
      cout << "Citizens win" << endl;
      exit(EXIT_SUCCESS);
    }

    usleep(500);
  }
}
