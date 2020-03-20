//
// Created by wu on 3/19/20.
//

#ifndef SIMPLEWEBSERVER_SERVER_H
#define SIMPLEWEBSERVER_SERVER_H


#include "base/noncopyable.h"
#include "EventLoopThreadPool.h"


class Server: noncopyable{
public:
    Server(EventLoop* loop,int ThreadNum,int port);
    ~Server();

    void Start();

    void HandleAccept();
    void HandleSelf(){ m_loop->UpdatePoller(m_ListenChannel,0);} // 更新事件
private:

    int m_threadNum; // 线程池线程数
    bool Started;
    std::unique_ptr<EventLoopThreadPool> m_ThreadPool;
    EventLoop* m_loop;
    int m_Listenfd;
    int m_port;
    SPChannel m_ListenChannel; // 对应监听事件的Channel
    static const int MAXFDS = 100000;
};


#endif //SIMPLEWEBSERVER_SERVER_H
