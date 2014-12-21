#pragma once

using namespace std;

class lock_free_mutex {
  private:
    atomic_flag flag;

  public:
    lock_free_mutex():
      flag{ATOMIC_FLAG_INIT}
    {}

    void lock() {
      while (flag.test_and_set(memory_order_acquire));
    }

    void unlock() {
      flag.clear(memory_order_release);
    }
};
