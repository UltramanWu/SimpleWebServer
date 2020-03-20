//
// Created by wu on 3/19/20.
//

#ifndef SIMPLEWEBSERVER_UTIL_H
#define SIMPLEWEBSERVER_UTIL_H


#include <cstdio>
#include <unistd.h>
#include <errno.h>
#include <string>
#include <signal.h>
#include <netinet/tcp.h> // 设置TCPNODELAY头文件
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <memory.h>

const int MAXBUFF = 4096;

namespace Util {
    ssize_t Readn(int fd,void* buf,size_t len);

    ssize_t Readn(int fd,std::string &buf,bool &zero);


    ssize_t Writen(int fd,void *buf,size_t len);

    ssize_t Writen(int fd,std::string &buf);

    void HandleForSigPipe(int fd);

    int SetListenAndBindPort(int port);

    int SetNonBlock(int &fd);

    // 禁用Nagle算法
    void SetSocketNoDelay(int &fd);

    void ShutDownWR(int fd);
}


#endif //SIMPLEWEBSERVER_UTIL_H
