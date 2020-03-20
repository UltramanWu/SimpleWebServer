//
// Created by wu on 3/19/20.
//

#ifndef SIMPLEWEBSERVER_EVENTLOOPTHREADPOOL_H
#define SIMPLEWEBSERVER_EVENTLOOPTHREADPOOL_H


#include <vector>
#include "EventLoopThread.h"

class EventLoopThreadPool {
public:
    EventLoopThreadPool(EventLoop* loop,int ThreadNum);
    ~EventLoopThreadPool();

    void Start();
    EventLoop* GetNextLoop();

private:
    EventLoop* baseLoop; // 用于接受主循环，防止线程池中为0
    int m_ThreadNum; //线程数
    int next; // 获取下一分配的线程

    std::vector<std::shared_ptr<EventLoopThread>> m_threads; // 存放线程的容器
    std::vector<EventLoop*> m_loops; // 存放loop的容器，用于线程任务分配

    bool IsStarted;
};


#endif //SIMPLEWEBSERVER_EVENTLOOPTHREADPOOL_H
