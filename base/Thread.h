//
// Created by wu on 3/18/20.
//

#ifndef SIMPLEWEBSERVER_THREAD_H
#define SIMPLEWEBSERVER_THREAD_H


#include "MutexLock.h"
#include "Condition.h"
#include <thread>

typedef std::function<void()> Functor;

class Thread {
public:
    Thread(const Functor &functor);
    ~Thread();
    void start();
    void join();
    bool IsStart() { return IsStarted; }


private:
    Functor ThreadFunc;
    std::unique_ptr<std::thread> m_thread;

    bool IsStarted;
    bool IsJoined;

};


#endif //SIMPLEWEBSERVER_THREAD_H
