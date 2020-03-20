//
// Created by wu on 3/19/20.
//

#include "EventLoopThreadPool.h"
EventLoopThreadPool::EventLoopThreadPool(EventLoop *loop, int ThreadNum):baseLoop(loop),IsStarted(false),
                                                                         m_ThreadNum(ThreadNum),next(0)
{

}

EventLoopThreadPool::~EventLoopThreadPool() {

}

void EventLoopThreadPool::Start() {
    baseLoop->assertIsInLoopThread();
    IsStarted = true;
    for(int i=0;i<m_ThreadNum;++i){ // 创建线程并提交运行
        std::shared_ptr<EventLoopThread> t(std::make_shared<EventLoopThread>());
        m_threads.emplace_back(t);
        m_loops.emplace_back(t->StartLoop());
    }
}


EventLoop* EventLoopThreadPool::GetNextLoop() {
    baseLoop->assertIsInLoopThread();
    assert(IsStarted);
    EventLoop* loop = baseLoop;
    if(!m_loops.empty()){
        loop= m_loops[next];
        next =(next+1)%m_ThreadNum;
    }
    return loop;
}

