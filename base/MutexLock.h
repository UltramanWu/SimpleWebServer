//
// Created by wu on 3/18/20.
//

#ifndef SIMPLEWEBSERVER_MUTEXLOCK_H
#define SIMPLEWEBSERVER_MUTEXLOCK_H

#include <pthread.h>
#include "noncopyable.h"

class MutexLock:noncopyable{
public:
    MutexLock(){
        pthread_mutex_init(&m_mutex,NULL);
    }
    ~MutexLock(){
        pthread_mutex_lock(&m_mutex);
        pthread_mutex_destroy(&m_mutex);
    }

    pthread_mutex_t* get(){ return &m_mutex;}
    void lock(){ pthread_mutex_lock(&m_mutex); }
    void unlock(){ pthread_mutex_unlock(&m_mutex); }

private:
    pthread_mutex_t m_mutex;
};


class LockGuard:noncopyable{
public:
    explicit LockGuard(MutexLock& lock):m_lock(lock) { m_lock.lock(); }
    ~LockGuard(){ m_lock.unlock(); }
private:
    MutexLock &m_lock;
};

#endif //SIMPLEWEBSERVER_MUTEXLOCK_H
