//
// Created by wu on 3/18/20.
//

#include "EventLoop.h"
#include "base/CurrentThread.h"
#include <sys/eventfd.h>
#include <iostream>
#include <sys/epoll.h>

__thread EventLoop* t_loopInThisThread = nullptr;

int CreateEventfd(){
    int evtfd = eventfd(0,EFD_NONBLOCK|EFD_CLOEXEC);
    if(evtfd < 0){
        abort();
    }
    return evtfd;
}


EventLoop::EventLoop():m_Poller(new Poller()),m_ThreadId(CurrentThread::tid()),m_WakeUpfd(CreateEventfd()),
                       looping(false),doPendingFunctors(false),quit(false),eventHanding(false),
                       pWakeupChannel(std::make_shared<Channel>(this,m_WakeUpfd))
{

    if(t_loopInThisThread){
        std::cout<<"EventLoop Created"<<"in thread "<<m_ThreadId;
    } else{
        t_loopInThisThread = this;
    }

    pWakeupChannel->SetEvent(EPOLLIN|EPOLLET);
    pWakeupChannel->SetFd(m_WakeUpfd);
    // 加入到Poller中
    pWakeupChannel->SetReadCallBack(std::bind(&EventLoop::HandleRead,this));
    pWakeupChannel->SetConnCallBack(std::bind(&EventLoop::HandleConn,this));
    m_Poller->Poller_Add(pWakeupChannel,0);

}

EventLoop::~EventLoop() {
    close(m_WakeUpfd); // 析构时注意关闭fd
    t_loopInThisThread = nullptr;
}

void EventLoop::RunInLoop(Functor &&cb) {
    if(IsInLoopThread())
        cb();
    else
        QueueInLoop(std::move(cb)); // 将其加入到队列中
}

void EventLoop::QueueInLoop(Functor &&cb) {
    {
        LockGuard lock(m_mutex);
        m_PendingFunctors.emplace_back(std::move(cb));
    }
    // 如果不是本线程调用或者正在处理阻塞函数的话，则直接WakeUp将该线程唤醒
    if(!IsInLoopThread()||doPendingFunctors) WakeUp();
}

void EventLoop::HandleRead() {
    uint64_t one = 1;
    int n = read(m_WakeUpfd,&one,sizeof(one));
    if(n<8){
        std::cout<<"EventLoop::WakeUp Write "<<n<<"Bytes instead of 8";
    }
    pWakeupChannel->SetEvent(EPOLLIN|EPOLLET);
}

void EventLoop::HandleConn() {
    m_Poller->Poller_Mod(pWakeupChannel,0);
}

void EventLoop::WakeUp() {
    uint64_t one = 1;
    int n = write(m_WakeUpfd,&one,sizeof(one));
    if(n<8){
        std::cout<<"EventLoop::WakeUp Write "<<n<<"Bytes instead of 8";
    }
}

void EventLoop::Quit() {
    quit = true;
    if(!IsInLoopThread()){
        WakeUp();
    }
}

void EventLoop::Loop() {
    assert(!looping);
    assert(!quit);
    looping = true;
    quit = false;

    while(!quit){
        m_ActiveChans.clear();
        m_Poller->Poll(m_ActiveChans);
        eventHanding = true;
        for(auto &it:m_ActiveChans) it->EventHandler();
        eventHanding = false;
        DoPendingFunc(); // 处理阻塞函数
        m_Poller->HandleExpired(); // 处理超时事件
    }
    looping = false;
}

void EventLoop::AddPoller(SPChannel Request, int timeout) {
    m_Poller->Poller_Add(Request,timeout);
}

void EventLoop::UpdatePoller(SPChannel Request, int timeout) {
    m_Poller->Poller_Mod(Request,timeout);
}

void EventLoop::RemovePoller(SPChannel Request) {
    m_Poller->Poller_Del(Request);
}

void EventLoop::DoPendingFunc() {
    std::vector<Functor> PendingFuns;
    doPendingFunctors = true;
    {
        LockGuard lock(m_mutex);
        PendingFuns.swap(m_PendingFunctors);
    }
    for(auto &it:PendingFuns) it();
    doPendingFunctors = false;
}


