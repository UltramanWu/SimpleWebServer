//
// Created by wu on 3/18/20.
//

#include "Channel.h"

Channel::Channel(EventLoop *loop, int fd):m_loop(loop),m_fd(fd),m_event(0),m_revent(0),m_status(New)
{}

Channel::Channel(EventLoop *loop):Channel(loop,0){} // 委托构造

// 事件处理函数
void Channel::EventHandler() {

    if(m_revent&EPOLLERR) {
        if(ErrorCallBack) ErrorCallBack();
        m_revent = 0;
        return;
    }
    if(m_revent&ReadEvent) {
        if (ReadCallBack) ReadCallBack();
    }

    if(m_revent&WriteEvent){
        if(WriteCallBack) WriteCallBack();
    }
    m_revent = 0;
    if(ConnCallBack)
        ConnCallBack();
}