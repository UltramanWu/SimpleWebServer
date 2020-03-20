//
// Created by wu on 3/18/20.
//

#ifndef SIMPLEWEBSERVER_COUNTDOWNLATCH_H
#define SIMPLEWEBSERVER_COUNTDOWNLATCH_H


#include "MutexLock.h"
#include "Condition.h"

class CountDownLatch {
public:
    explicit CountDownLatch(int count);
    ~CountDownLatch();
    void wait();
    void countDown();
private:
    MutexLock m_mutex;
    Condition m_cond;
    int m_count;
};


#endif //SIMPLEWEBSERVER_COUNTDOWNLATCH_H
