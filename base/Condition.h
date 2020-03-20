//
// Created by wu on 3/18/20.
//

#ifndef SIMPLEWEBSERVER_CONDITION_H
#define SIMPLEWEBSERVER_CONDITION_H

#include <errno.h>
#include "MutexLock.h"

class Condition:noncopyable{
public:
    explicit Condition(MutexLock& lock):m_lock(lock){ pthread_cond_init(&m_cond,NULL); }
    ~Condition(){ pthread_cond_destroy(&m_cond); }

    void Wait(){
        pthread_cond_wait(&m_cond,m_lock.get());
    }

    bool WaitForSecond(int sec){
        struct timespec abstime;
        clock_gettime(CLOCK_REALTIME,&abstime);
        abstime.tv_sec += static_cast<__time_t>(sec);
        return ETIMEDOUT==pthread_cond_timedwait(&m_cond,m_lock.get(),&abstime);
    }

    void notify(){
        pthread_cond_signal(&m_cond);
    }

    void notifyAll(){
        pthread_cond_broadcast(&m_cond);
    }
private:
    MutexLock &m_lock;
    pthread_cond_t m_cond;
};

#endif //SIMPLEWEBSERVER_CONDITION_H
