//
// Created by wu on 3/18/20.
//

#include "Poller.h"
#include <sys/epoll.h>
#include <iostream>
#include <assert.h>

const int EVENTNUMS = 4096;
const int EPOLL_WAITTIME = 10000;

Poller::Poller():m_epollfd(epoll_create1(EPOLL_CLOEXEC)),m_ActiveEvents(EVENTNUMS){
    assert(m_epollfd>0);
}

Poller::~Poller() { close(m_epollfd); }

void Poller::Poll(std::vector<SPChannel> &ActiveChans) {

        int eventNum = epoll_wait(m_epollfd,&*m_ActiveEvents.begin(),m_ActiveEvents.size(),EPOLL_WAITTIME);
        if(eventNum<0){
            perror("Poller::Poll eventNum < 0");
            abort();
        }
        else if(eventNum == 0){
            //perror("Poller::Poll eventNum == 0");
        }
        else{
            GetRequest(eventNum,ActiveChans);
            if(ActiveChans.size()>0) return;
        }


}

// 填充Channel数组用于返回
void Poller::GetRequest(int eventNum, std::vector<SPChannel> &ActiveChans) {
    for(int i=0;i<eventNum;++i){
        int fd = m_ActiveEvents[i].data.fd;
        SPChannel req = m_fd2Chans[fd];
        if(req){
            req->SetRevent(req->GetEvent());
            req->SetEvent(0);
            ActiveChans.emplace_back(req);
        }else{
            std::cout<<"SP cur_req is invalid!"<<std::endl;
        }
    }
}

void Poller::Poller_Add(SPChannel request, int timeout) {
    if(request->GetPollStatus() == New){
        int fd = request->Getfd();
        // std::cout<<"fd "<<fd<<" is Add Poller"<<std::endl;
        assert(m_fd2Chans[fd]== nullptr);
        if(timeout>0){
            SPHttpData httpData = request->GetHttp();
            Add_Timer(httpData,timeout);
            m_fd2Https[fd] = httpData;
        }

        struct epoll_event event;
        event.data.fd = fd;
        event.events = request->GetEvent();
        if(epoll_ctl(m_epollfd,EPOLL_CTL_ADD,fd,&event)<0){
            std::cout<<"Poller_Add failed!"<<std::endl;
            m_fd2Https[fd].reset();
            return;
        }
        request->SetPollStatus(Added);
        m_fd2Chans[fd] = request;
    }
}

void Poller::Poller_Mod(SPChannel request, int timeout) {
    if(request->GetPollStatus() == Added){
        int fd = request->Getfd();
        // std::cout<<"fd "<<fd<<" is Poller_Mod"<<std::endl;
        if(timeout > 0){
            SPHttpData httpData = request->GetHttp();
            Add_Timer(httpData,timeout);
            m_fd2Https[fd] = httpData;
        }
//        std::cout<<"m_fd2Chans[fd] is "<< m_fd2Chans[fd]<<std::endl;
//        std::cout<<"request is "<< request<<std::endl;
        assert(m_fd2Chans[fd]==request);

        struct epoll_event event;
        event.data.fd = fd;
        event.events = request->GetEvent();
        if(epoll_ctl(m_epollfd,EPOLL_CTL_MOD,fd,&event)<0){
            std::cout<<"Poller_Add failed!"<<std::endl;
            m_fd2Chans[fd].reset();
            m_fd2Https[fd].reset();
        }

    }else if(request->GetPollStatus() == New){
        std::cout<<"the request "<< request.get()<<" is New"<<std::endl;
    }
}

void Poller::Poller_Del(SPChannel request) {
    if(request->GetPollStatus() == Added){
        int fd = request->Getfd();
        assert(m_fd2Chans[fd]==request);

        struct epoll_event event;
        event.data.fd = fd;
        event.events = request->GetEvent();

        if(epoll_ctl(m_epollfd,EPOLL_CTL_DEL,fd,&event)<0){
            std::cout<<"Poller_Add failed!"<<std::endl;
        }
        m_fd2Chans[fd].reset();
        m_fd2Https[fd].reset();
        request->SetPollStatus(Deleted);
    }
}

void Poller::Add_Timer(SPHttpData request, int timeout) {
    m_timerQ.AddTimer(request,timeout);
}


