//
// Created by wu on 3/18/20.
//

#ifndef SIMPLEWEBSERVER_POLLER_H
#define SIMPLEWEBSERVER_POLLER_H

#include <sys/epoll.h> // epoll头文件
#include <vector>
#include "EventLoop.h"
#include "Channel.h"
#include "TimerQueue.h"

class TimerQueue;
class Channel;
class HttpData;
typedef std::shared_ptr<Channel> SPChannel;
typedef std::shared_ptr<HttpData> SPHttpData;


class Poller {
public:
    Poller();
    ~Poller();

    void Poller_Add(SPChannel request,int timeout);
    void Poller_Mod(SPChannel request,int timeout);
    void Poller_Del(SPChannel request);

    void Add_Timer(SPHttpData request,int timeout);

    void Poll(std::vector<SPChannel>& ActiveChans);

    void HandleExpired() { m_timerQ.HandleExpired(); }

    void GetRequest(int eventNum,std::vector<SPChannel>& ActiveChans);

private:
    static const int MAXFDS = 10000;
    int m_epollfd;
    std::vector<epoll_event> m_ActiveEvents; // 用于填充SPChannel的容器

    SPChannel m_fd2Chans[MAXFDS]; // 用于fd与Channel之间一一对应

    SPHttpData m_fd2Https[MAXFDS]; // 用于fd与HttpData之间一一对应

    TimerQueue m_timerQ; // 定时器队列，用于填充超时管理
};


#endif //SIMPLEWEBSERVER_POLLER_H
