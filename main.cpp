#include <iostream>
#include "Server.h"
#include <getopt.h>

int main(int argc,char* argv[]) {
    int ThreadNum = 4;
    int port = 8080;

    int opt;
    std::string match = "l:t:p:";
    while((opt = getopt(argc,argv,match.c_str()))>0){
        switch (opt){
            case 'l':
            {
                break;
            }
            case 't':
            {
                ThreadNum = atoi(optarg);
             break;
            }
            case 'p':
            {
                port = atoi(optarg);
                break;
            }
            default:
                break;
        }
    }

    // 主循环
    EventLoop mainLoop;
    mainLoop.assertIsInLoopThread();
    Server HttpServer(&mainLoop,ThreadNum,port);
    HttpServer.Start();
    mainLoop.Loop();

    return 0;
}