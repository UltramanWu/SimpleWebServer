//
// Created by wu on 3/19/20.
//

#ifndef SIMPLEWEBSERVER_HTTPDATA_H
#define SIMPLEWEBSERVER_HTTPDATA_H

#include <map>
#include "Channel.h"
#include "EventLoop.h"
#include "TimerQueue.h"
class TimerNode;

typedef enum { PARSE_INVALID,PARSE_LINE,PARSE_HEADER,PARSE_ANALYSIS,PARSE_BODY,PARSE_SUCCESS }ParseState;
typedef enum { LINE_SUCCESS,LINE_ERROR,LINE_AGAIN }LineState;
typedef enum { HEADER_SUCCESS,HEADER_ERROR,HEADER_AGAIN }HeaderState;
typedef enum { RESPONSE_SUCCESS,RESPONSE_ERROR }ResponseState;
typedef enum { BODY_SUCCESS,BODY_ERROR }BodyState;
typedef enum { VERUNKONWN,HTTP_10,HTTP_11 }HttpVersion;
typedef enum { HTTPUNKONWN,GET,HEAD,POST }HttpMethod;
typedef enum { CONNECTED,DISCONNECTING,DISCONNCTED }ConnectState;

typedef enum { STATUS_200OK,STATUS_301MOVEPER,STATUS_400BADREQUEST,
    STATUS403FORBIDDEN,STATUS404NOTFOUND,STATU500INTELERROR,STATUS50SERVERAVALIABLE } StatusCode;

class EventLoop;
class Channel;
typedef std::shared_ptr<Channel> SPChannel;

class MimeType {
private:
    static void init();
    static std::map<std::string, std::string> mime;
    MimeType();
    MimeType(const MimeType &m);

public:
    static std::string getMime(const std::string &suffix);

private:
    static pthread_once_t once_control;
};


class HttpData:public std::enable_shared_from_this<HttpData> {
public:
    HttpData(EventLoop *loop,int fd);
    ~HttpData();
    void NewEvent();
    SPChannel GetChannel(){ return m_Channel; }
    void LinkTimer(std::shared_ptr<TimerNode> timer) { m_timer = timer; }

    void ReadHandle(); // 读取处理
    void WriteHandle(); // 写入处理
    void ErrorHandle(int fd,int code,std::string msg); // 错误处理
    void ConnHandle(); // 连接处理
    void CloseHandle(); // 关闭处理

private:
    void Reset();
    void SeparateTimer();



    LineState ProcessRequestLine(); // 解析请求行
    HeaderState ProcessRequestHeader(); // 解析请求头
    ResponseState ProcessResponse(); // 解析请求体


    // 缓冲区
    std::string In_Buffer;
    std::string Out_Buffer;

    int m_fd; //accept事件
    SPChannel m_Channel; // Channel对象
    EventLoop* m_loop; // EventLoop对象
    bool error;

    std::string m_filename; // 文件名
    ParseState m_ParseState; // 文件解析状态
    HttpVersion m_version; // HTTP版本
    HttpMethod m_method; // HTTP方法
    ConnectState m_conn; // 连接状态

    std::weak_ptr<TimerNode> m_timer;
    std::map<std::string,std::string> headers; // 用于请求头部一一对应

    bool KeepAlive;

};


#endif //SIMPLEWEBSERVER_HTTPDATA_H
