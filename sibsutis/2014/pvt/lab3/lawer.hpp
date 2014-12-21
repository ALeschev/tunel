#pragma once

using namespace std;

extern deque <ticket*> queue;
extern mutex queue_mutex;

class lawer {
  private:
    int time;
    int tickets_fail;
    int tickets_success;
    bool active;

  public:
    lawer() {
      time = (rand() % 5) + 1;
      tickets_fail = 0;
      tickets_success = 0;
      active = true;
    }

    bool time_to_leave(int limit) {
      return tickets_fail - tickets_success > limit;
    }

    void leave() {
      active = false;
    }

    bool status() {
      return active;
    }

    void get_task() {
      unique_lock <mutex> lock(queue_mutex);
      ticket *task;

      if (queue.empty()) {
        return;
      }

      task = queue.front();
      queue.pop_front();
      lock.unlock();

      task->promise_time = time;
      task->real_time = rand() % (time * 2);
      sleep(task->real_time);
      task->task.set_value(true);

      if (task->real_time > task->promise_time) {
        tickets_fail++;
      }
      else {
        tickets_success++;
      }
    }
};
