//
// Created by wu on 3/19/20.
//

#include "Util.h"

namespace Util {
    ssize_t Readn(int fd,void* buf,size_t len){
        size_t nleft = len;
        ssize_t ReadNum = 0;
        ssize_t ReadSum = 0;

        char* ptr = (char*)buf;
        while(nleft > 0){
            ReadNum=read(fd,ptr,nleft);
            if(ReadNum < 0){
                if(errno == EINTR) // 防止被gdb打断
                    continue;
                else if(errno == EAGAIN)
                    return ReadSum;
                else
                    return -1;
            }else if(ReadNum == 0){
                break;
            }

            nleft -= ReadNum;
            ReadSum += ReadNum;
            ptr += ReadNum;
        }
        return ReadSum;
    }

    ssize_t Readn(int fd,std::string &buf,bool &zero){
        ssize_t ReadNum = 0;
        ssize_t ReadSum = 0;
        while(true){
            char buff[MAXBUFF];
            ReadNum = read(fd,buff,sizeof(buff));
            if(ReadNum < 0){
                if(errno == EINTR)
                    continue;
                else if(errno == EAGAIN)
                    break;
                else
                {
                    perror("Read Error!");
                    return -1;
                }
            }else if(ReadNum == 0){
                zero = true;
                break;
            }
            ReadSum += ReadNum;
            buf += std::string(buff,buff+ReadNum);
        }
        return ReadSum;
    }


    ssize_t Writen(int fd,void *buf,size_t len){
        size_t nleft = len;
        ssize_t WriteNum = 0;
        ssize_t WriteSum = 0;
        char* ptr = (char*) buf;
        while(nleft>0){
            WriteNum = write(fd,ptr,nleft);
            if(WriteNum<0){
                if(errno == EINTR)
                    continue;
                else if(errno == EAGAIN)
                    break;
                else
                    return -1;
            }
            WriteSum += WriteNum;
            ptr += WriteNum;
            nleft -= WriteNum;
        }
        return WriteSum;
    }

    ssize_t Writen(int fd,std::string &buf){
        size_t nleft = buf.size();
        ssize_t WriteNum = 0;
        ssize_t WriteSum = 0;
        char* ptr = (char*) buf.c_str();
        while(nleft>0){
            WriteNum = write(fd,ptr,nleft);
            if(WriteNum<0){
                if(errno == EINTR)
                    continue;
                else if(errno == EAGAIN)
                    break;
                else
                    return -1;
            }
            WriteSum += WriteNum;
            ptr += WriteNum;
            nleft -= WriteNum;
        }
        // 截取字符串内容
        if(WriteSum == buf.size())
            buf.clear();
        else
            buf = buf.substr(WriteSum);
        return WriteSum;
    }

    void HandleForSigPipe(int fd){
        signal(SIGPIPE,SIG_IGN);
    }

    int SetListenAndBindPort(int port){
        // 检查port取值区间
        if(port < 0||port>65535) return -1;

        int listenfd;
        if((listenfd = socket(AF_INET,SOCK_STREAM,0)) == -1) return -1;

        int optval = 1;
        if(setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&optval, sizeof(int)) == -1){
            close(listenfd);
            return -1;
        }

        // 设置服务器IP和PORT，与listenfd绑定
        sockaddr_in ServerAddr;
        bzero(&ServerAddr, sizeof(ServerAddr));

        ServerAddr.sin_family = AF_INET;
        ServerAddr.sin_addr.s_addr = htonl(INADDR_ANY); // 监听所有端口
        ServerAddr.sin_port = htons(port);

        if((bind(listenfd,(sockaddr*)&ServerAddr, sizeof(sockaddr))) == -1){
            close(listenfd);
            return -1;
        }

        // 开始监听，将backlog进行设置
        if(listen(listenfd,2048) == -1){
            close(listenfd);
            return -1;
        }
        return listenfd;
    }

    int SetNonBlock(int &fd){
        int flag = fcntl(fd,F_GETFL,0);
        if(flag == -1) return -1;
        flag |= O_NONBLOCK;
        if(fcntl(fd,F_SETFL,flag) == -1) return -1;
        return 0;
    }

    // 禁用Nagle算法
    void SetSocketNoDelay(int &fd){
        int enable = 1;
        setsockopt(fd,IPPROTO_TCP,TCP_NODELAY,&enable,sizeof(int));
    }

    void ShutDownWR(int fd){
        shutdown(fd,SHUT_WR);
    }
}
