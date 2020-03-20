//
// Created by wu on 3/18/20.
//

#include <assert.h>
#include "EventLoopThread.h"

EventLoopThread::EventLoopThread():m_loop(nullptr),m_mutex(),m_cond(m_mutex),
                                   m_thread(std::bind(&EventLoopThread::ThreadFun,this))
{}

EventLoopThread::~EventLoopThread() {
    if(m_loop){
        m_loop->Quit();
        m_thread.join();
    }
}

EventLoop* EventLoopThread::StartLoop() {
    assert(!m_thread.IsStart());
    m_thread.start();
    {
        LockGuard lock(m_mutex);
        while(m_loop==nullptr) m_cond.Wait(); // 如果Loop不存在的话，则等待
    }
    return m_loop;
}

void EventLoopThread::ThreadFun() { // 线程函数运行后，获取到Loop对象，并条件变量通知StartLoop返回
    EventLoop EvtLoop;
    {
        LockGuard lock(m_mutex);
        m_loop = &EvtLoop;
        m_cond.notify();
    }
    EvtLoop.Loop();

    LockGuard lock(m_mutex);
    m_loop = nullptr;
}