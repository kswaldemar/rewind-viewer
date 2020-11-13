//
// Created by Vladimir A. Kiselev on 08.11.2020.
//

#pragma once

#include <atomic>

class Spinlock {
 public:
    void lock();
    void unlock();

 private:
    std::atomic_flag lock_ = ATOMIC_FLAG_INIT;
};
