//
// Created by wu on 3/20/20.
//

#include <iostream>
#include <cstdio>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include <errno.h>

using namespace std;

const int MAXSIZE = 1024;
const char* SERVERRESS = "127.0.0.1";
const int SERVPORT = 8080;
const int FDSIZE = 1024;
const int EPOLLEVENTS = 20;

int SetNonBlock(int &fd){
    int flag = fcntl(fd,F_GETFL,0);
    if(flag == -1) return -1;
    flag |=O_NONBLOCK;
    if(fcntl(fd,F_SETFL,flag) == -1) return -1;

    return 0;
}


int main(){

    int sockfd;
    sockaddr_in ServerAddr;
    ServerAddr.sin_family = AF_INET;
    ServerAddr.sin_port = htons(8080);
    ServerAddr.sin_addr.s_addr = inet_addr(SERVERRESS);

    const char* p = "";

    char buff[4096];
    buff[0] = '\0';
    ssize_t n;
    for(int i=0;i<1000;i++){
        p = " ";
        sockfd = socket(AF_INET,SOCK_STREAM,0);
        if(connect(sockfd,(sockaddr*)&ServerAddr, sizeof(ServerAddr)) == 0){
            //SetNonBlock(sockfd);
            cout<<"1: "<<endl;
            n = write(sockfd,p,strlen(p));

            cout<<"strlen(p) = "<<strlen(p)<<endl;
            cout<<"Send Byte is "<< n<<endl;

            //sleep(1);
            bzero(buff, sizeof(buff));
            n = read(sockfd,buff,sizeof(buff));
            cout<<"Read Byte is "<< n<<endl;
            printf("%s",buff);
            cout<<"errno is "<<errno<<endl;
        }
        else{
            cout<<"connect 1 is falied!"<<endl;
        }
        close(sockfd);
        //sleep(1);
    }


    p = "GET / HTTP/1.1";
    sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(connect(sockfd,(sockaddr*)&ServerAddr, sizeof(ServerAddr)) == 0){
        SetNonBlock(sockfd);
        cout<<"2: "<<endl;
        n = write(sockfd,p,strlen(p));

        cout<<"strlen(p) = "<<strlen(p)<<endl;
        cout<<"Send Byte is "<< n<<endl;

        sleep(1);
        bzero(buff, sizeof(buff));
        n = read(sockfd,buff,sizeof(buff));
        cout<<"Read Byte is "<< n<<endl;
        printf("%s",buff);
        cout<<"errno is "<<errno<<endl;
    }
    else{
        cout<<"connect 2 is falied!"<<endl;
    }
    close(sockfd);
    sleep(1);

    p = "GET /hello HTTP/1.1\r\nhost:192.168.0.105:8080\r\nContent-Type:"
        "Application/x-www-form-urlencoded\r\nConnection:Keep-Alive\r\n\r\n";
    sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(connect(sockfd,(sockaddr*)&ServerAddr, sizeof(ServerAddr)) == 0){
        SetNonBlock(sockfd);
        cout<<"3: "<<endl;
        n = write(sockfd,p,strlen(p));

        cout<<"strlen(p) = "<<strlen(p)<<endl;
        cout<<"Send Byte is "<< n<<endl;

        sleep(1);
        bzero(buff, sizeof(buff));
        n = read(sockfd,buff,sizeof(buff));
        cout<<"Read Byte is "<< n<<endl;
        printf("%s",buff);
        cout<<"errno is "<<errno<<endl;
    }
    else{
        cout<<"connect 3 is falied!"<<endl;
    }
    close(sockfd);
    sleep(1);


    return 0;
}