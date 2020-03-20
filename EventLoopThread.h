//
// Created by wu on 3/18/20.
//

#ifndef SIMPLEWEBSERVER_EVENTLOOPTHREAD_H
#define SIMPLEWEBSERVER_EVENTLOOPTHREAD_H


#include "base/Thread.h"
#include "EventLoop.h"

class EventLoop;

class EventLoopThread {
public:
    EventLoopThread();
    ~EventLoopThread();

    EventLoop* StartLoop();

private:
    void ThreadFun(); // 线程函数
    EventLoop* m_loop;
    Thread m_thread;
    Condition m_cond;
    MutexLock m_mutex;
};


#endif //SIMPLEWEBSERVER_EVENTLOOPTHREAD_H
