//
// Created by wu on 3/19/20.
//

#include <iostream>
#include <arpa/inet.h>
#include "Server.h"
#include "Util.h"
#include "HttpData.h"

Server::Server(EventLoop *loop, int ThreadNum, int port):m_loop(loop),m_Listenfd(Util::SetListenAndBindPort(port)),
                                                         m_threadNum(ThreadNum),m_port(port),Started(false),
                                                         m_ListenChannel(std::make_shared<Channel>(loop,m_Listenfd)),
                                                         m_ThreadPool(new EventLoopThreadPool(loop,ThreadNum))
{
    assert(m_Listenfd>0);
    assert(m_ListenChannel->Getfd() == m_Listenfd);
    std::cout<<"listenfd "<<m_ListenChannel->Getfd()<<" is Created"<<std::endl;
    // 设置m_listenfd为非阻塞
    if(Util::SetNonBlock(m_Listenfd)<0){
        std::cout<<"Listenfd "<<m_Listenfd<<" SetNonBlock failed"<<std::endl;
    }

    Util::HandleForSigPipe(m_Listenfd); // 忽略SIGPIPE信号

}

Server::~Server() {
    close(m_Listenfd);
}

void Server::Start() {
    assert(!Started);
    m_ThreadPool->Start();

    m_ListenChannel->SetEvent(EPOLLIN|EPOLLET); // 设置读触发和边沿触发
    m_ListenChannel->SetReadCallBack(std::bind(&Server::HandleAccept,this));
    m_ListenChannel->SetConnCallBack(std::bind(&Server::HandleSelf,this));
    m_loop->AddPoller(m_ListenChannel,0);

    Started = true;
}

void Server::HandleAccept() {
    sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(sockaddr_in);
    int acceptfd;
    while((acceptfd=::accept(m_Listenfd,(sockaddr*)&clientAddr,&clientAddrLen)) > 0){

        EventLoop* loop = m_ThreadPool->GetNextLoop();
        std::cout<<"NextLoop is "<<loop<<std::endl;
        std::cout<<"NewEvent is from "<< inet_ntoa(clientAddr.sin_addr)<<":"<<ntohs(clientAddr.sin_port)<<std::endl;
        std::cout<<"acceptfd is "<<acceptfd<<std::endl;
        // 如果acceptfd大于最大接受事件数的话，直接关闭
        // 否则会服务端会发送RST信号
        if(acceptfd>=MAXFDS) {
            close(acceptfd);
            continue;
        }

        if(Util::SetNonBlock(acceptfd)<0){
            std::cout<<"acceptfd "<<acceptfd<<"SetNonBlock failed!"<<std::endl;
            return;
        }
        Util::SetSocketNoDelay(acceptfd); // 禁用Nagle算法,测试过程为小包发送


        SPHttpData req_info(std::make_shared<HttpData>(loop,acceptfd));
        req_info->GetChannel()->SetHttp(req_info); // 设定Channel的值
        loop->QueueInLoop(std::bind(&HttpData::NewEvent,req_info));

    }
    m_ListenChannel->SetEvent(EPOLLIN|EPOLLET); // 在监听完一组后需要重新更新listenfd，否则将不再监听事件
}
