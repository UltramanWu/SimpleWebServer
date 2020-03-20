//
// Created by wu on 3/18/20.
//

#ifndef SIMPLEWEBSERVER_CHANNEL_H
#define SIMPLEWEBSERVER_CHANNEL_H

#include <functional>
#include <sys/epoll.h>
#include "EventLoop.h"
#include "HttpData.h"

typedef std::function<void()> CallBack; // 回调函数模板
typedef std::shared_ptr<HttpData> SPHttpData;
typedef enum { New,Added,Deleted } PollStatus; // 本Channel的添加状态
class EventLoop;


class Channel {
public:
    Channel(EventLoop* loop,int fd);
    Channel(EventLoop* loop);
    ~Channel(){}

    void SetReadCallBack(CallBack &&cb){ ReadCallBack=cb; }
    void SetWriteCallBack(CallBack &&cb){ WriteCallBack=cb; }
    void SetErrorCallBack(CallBack &&cb){ ErrorCallBack=cb; }
    void SetConnCallBack(CallBack &&cb){ ConnCallBack=cb; }

    int Getfd() { return m_fd; }
    void SetFd(int fd) { m_fd = fd; }
    EventLoop* GetLoop(){ return m_loop; }

    uint32_t& GetEvent(){ return m_event; }
    void SetEvent(uint32_t ev) { m_event = ev; }
    void SetRevent(uint32_t ev){ m_revent = ev; }

    PollStatus GetPollStatus() { return m_status; }
    void SetPollStatus(PollStatus status) { m_status = status; }

    void EventHandler(); // 事件处理函数

    SPHttpData GetHttp() {
        if(!m_Http.expired()){ // 如果expired返回为true,则返回一个空指针
            return m_Http.lock();
        }
    }
    void SetHttp(SPHttpData httpData) { m_Http = httpData; }

private:
    // 回调函数
    CallBack ReadCallBack;
    CallBack WriteCallBack;
    CallBack ErrorCallBack;
    CallBack ConnCallBack;

    int m_fd;
    EventLoop* m_loop;
    std::weak_ptr<HttpData> m_Http;

    uint32_t m_event; // 准备事件
    uint32_t m_revent; // 就绪事件
    PollStatus m_status; // 在Poller中的状态

    static const uint32_t NonEvent = 0;
    static const uint32_t ReadEvent = EPOLLIN|EPOLLRDHUP|EPOLLPRI;
    static const uint32_t WriteEvent = EPOLLOUT;
};


#endif //SIMPLEWEBSERVER_CHANNEL_H
