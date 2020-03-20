//
// Created by wu on 3/18/20.
//

#ifndef SIMPLEWEBSERVER_EVENTLOOP_H
#define SIMPLEWEBSERVER_EVENTLOOP_H


#include "Channel.h"
#include <memory>
#include <vector>
#include <assert.h>
#include <iostream>
#include "Poller.h"
#include "base/MutexLock.h"
#include "base/CurrentThread.h"

class Channel;
class Poller;
typedef std::shared_ptr<Channel> SPChannel;
typedef std::function<void()> Functor;

class EventLoop {
public:
    EventLoop();
    ~EventLoop();
    void Loop();

    void AddPoller(SPChannel channel,int timeout);
    void UpdatePoller(SPChannel channel,int timeout);
    void RemovePoller(SPChannel channel);

    void Quit(); // 用于关闭循环，引起析构

    void RunInLoop(Functor &&cb);
    void QueueInLoop(Functor &&cb);
    void DoPendingFunc();

    void WakeUp(); // 用于唤醒进程

    bool IsInLoopThread() {
        return m_ThreadId==CurrentThread::tid(); }
    void assertIsInLoopThread() { assert(IsInLoopThread()); }

private:
    int m_WakeUpfd;
    SPChannel pWakeupChannel;
    std::unique_ptr<Poller> m_Poller; // IO复用选择器
    std::vector<SPChannel> m_ActiveChans; // 用于就绪Channel并返回
    std::vector<Functor> m_PendingFunctors; // 用于填充阻塞任务

    const pid_t m_ThreadId; // 标记当前EventLoop对象对应的线程ID

    void HandleRead(); // 用于接受唤醒线程
    void HandleConn(); // 用于更新事件

    // 标志位
    bool looping;
    bool quit;
    bool doPendingFunctors;
    bool eventHanding;

    MutexLock m_mutex;
};


#endif //SIMPLEWEBSERVER_EVENTLOOP_H
