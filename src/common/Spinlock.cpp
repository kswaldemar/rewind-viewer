//
// Created by Vladimir A. Kiselev on 08.11.2020.
//

#include "Spinlock.h"

void Spinlock::lock() {
    while (lock_.test_and_set(std::memory_order_acquire))
        ;
}
void Spinlock::unlock() {
    lock_.clear(std::memory_order_release);
}
