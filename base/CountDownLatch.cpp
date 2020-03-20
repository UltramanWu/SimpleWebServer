//
// Created by wu on 3/18/20.
//

#include "CountDownLatch.h"

CountDownLatch::CountDownLatch(int count):m_count(count),m_cond(m_mutex),m_mutex()
{

}

void CountDownLatch::wait() {
    LockGuard lock(m_mutex);
    while(m_count>0) m_cond.Wait();
}

void CountDownLatch::countDown() {
    LockGuard lock(m_mutex);
    --m_count;
    if(m_count == 0) m_cond.notifyAll();
}