//
// Created by wu on 3/19/20.
//

#include <iostream>
#include "HttpData.h"
#include "Util.h"
#include <sys/mman.h> //mmap
#include <sys/stat.h> //stat

const int DEFAULT_EXPIRED_TIME = 2000; // 2s
const int DEFAULT_KEEPALIVE_TIME = 5*60*1000; // 5min

pthread_once_t MimeType::once_control = PTHREAD_ONCE_INIT;
std::map<std::string, std::string> MimeType::mime;

void MimeType::init() {
    mime[".html"] = "text/html";
    mime[".avi"] = "video/x-msvideo";
    mime[".bmp"] = "image/bmp";
    mime[".c"] = "text/plain";
    mime[".doc"] = "application/msword";
    mime[".gif"] = "image/gif";
    mime[".gz"] = "application/x-gzip";
    mime[".htm"] = "text/html";
    mime[".ico"] = "image/x-icon";
    mime[".jpg"] = "image/jpeg";
    mime[".png"] = "image/png";
    mime[".txt"] = "text/plain";
    mime[".mp3"] = "audio/mp3";
    mime["default"] = "text/html";
}


std::string MimeType::getMime(const std::string &suffix) {
    pthread_once(&once_control,MimeType::init);
    if(mime.find(suffix) == mime.end())
        return mime["default"];
    else
        return mime[suffix];
}



HttpData::HttpData(EventLoop *loop, int fd):m_loop(loop),m_fd(fd), m_Channel(std::make_shared<Channel>(loop,fd)),
                                            m_method(HTTPUNKONWN),m_version(VERUNKONWN),m_filename(""),KeepAlive(false),
                                            In_Buffer(""),Out_Buffer(""),error(false),m_conn(CONNECTED),m_ParseState(PARSE_INVALID)
{
    //std::cout<<"m_Channel->Getfd is"<< m_Channel->Getfd() <<std::endl;
    assert(m_Channel->Getfd() == fd);
    m_Channel->SetReadCallBack(std::bind(&HttpData::ReadHandle,this));
    m_Channel->SetWriteCallBack(std::bind(&HttpData::WriteHandle,this));
    m_Channel->SetConnCallBack(std::bind(&HttpData::ConnHandle,this));

}


void HttpData::NewEvent() {
    m_Channel->SetEvent(EPOLLIN|EPOLLET|EPOLLONESHOT); //EPOLLONESHOT只能触发一次，再次触发需要更新
    m_loop->AddPoller(m_Channel,DEFAULT_EXPIRED_TIME);
}

HttpData::~HttpData() {
    std::cout<<"fd "<< m_fd <<" ~HttpData"<<std::endl;
    m_Channel.reset(); // 当前的Channel指针需要置空
    close(m_fd);
    std::cout<<"fd "<< m_fd <<" closed"<<std::endl;
}

void HttpData::SeparateTimer() {
    if(!m_timer.expired()){
        std::shared_ptr<TimerNode> my_timer(m_timer.lock());
        my_timer->ClearReq();
        my_timer.reset();
    }
}

void HttpData::Reset() {
    m_filename.clear();
    m_method = HTTPUNKONWN;
    m_version = VERUNKONWN;
    m_ParseState = PARSE_INVALID;

    SeparateTimer(); // 分离定时器
}

void HttpData::ReadHandle() {
    uint32_t& event = m_Channel->GetEvent();
    ssize_t readSum;

    do{
        bool zero;
        readSum = Util::Readn(m_fd,In_Buffer,zero);

        std::cout<<"Request: "<<In_Buffer<<std::endl;
        std::cout<<"zero: "<<zero<<std::endl;
        std::cout<<"readSum: "<<readSum<<std::endl;
        if(readSum<0){
            perror("readSum < 0");
            error = true;
            break;
        }else if(zero){
            // 当读取到零值时，有两种情况，一种是还未到来
            // 第二种是对端已经关闭
            // 最有可能是 对端已经关闭，此时不直接关闭close，而是在写完后再关闭
            m_conn = DISCONNECTING;
            if(readSum == 0)
                break;
        }

        // 解析请求行
        if(m_ParseState == PARSE_INVALID){
            m_ParseState = PARSE_LINE;
            {
                LineState flag = ProcessRequestLine();
                std::cout<<"ProcessRequestLine request:"<<In_Buffer<<std::endl;
                if (flag == LINE_AGAIN) {
                    std::cout<<"fd "<< m_fd<<"this is LINE_AGAIN!"<<std::endl;
                    In_Buffer.clear();
                    break;
                } else if (flag == LINE_ERROR) {
                    std::cout<<"fd "<< m_fd<<"this is LINE_ERROR!"<<std::endl;
                    error = true;
                    In_Buffer.clear();
                    ErrorHandle(m_fd,400,"Bad Request");
                    break;
                }
            }

            m_ParseState = PARSE_HEADER;
            {
                HeaderState flag = ProcessRequestHeader();
                std::cout<<"ProcessRequestHeader request:"<<In_Buffer<<std::endl;
                if(flag == HEADER_AGAIN){
                    std::cout<<"fd "<< m_fd<<"this is HEADER_AGAIN!"<<std::endl;
                    In_Buffer.clear();
                    break;
                }
                else if(flag == HEADER_ERROR){
                    std::cout<<"fd "<< m_fd<<"this is HEADER_ERROR!"<<std::endl;
                    error = true;
                    In_Buffer.clear();
                    ErrorHandle(m_fd,400,"Bad Request");
                    break;
                }
            }

            if(m_method == POST){
                m_ParseState = PARSE_BODY;
            } else{
                m_ParseState = PARSE_ANALYSIS;
            }
            if(m_ParseState == PARSE_BODY){
                int ContentLength = 0;
                if(headers.find("Content-Length")!=headers.end()){
                    ContentLength = stoi(headers["Content-Length"]);
                }else{
                    error = true;
                    ErrorHandle(m_fd,400,"Bad Request:Lack of argument(Content-Length)");
                    break;
                }
                if(static_cast<int>(In_Buffer.size())<ContentLength) break;
                m_ParseState = PARSE_ANALYSIS;
            }

            if(m_ParseState == PARSE_ANALYSIS){
                ResponseState flag = ProcessResponse();
                if(flag == RESPONSE_SUCCESS){
                    m_ParseState = PARSE_SUCCESS;
                    break;
                }else{
                    error = true;
                    break;
                }
            }
        }
    }while(false);
    if(!error){ // 如果不出错误的话，继续操作
       if(Out_Buffer.size())
           WriteHandle();
       // 可能写操作会出现问题，该位置需要再次判断错误
       if(!error){
           Reset(); // 将解析状态回归初始化
           if(In_Buffer.size() && m_conn != DISCONNECTING) ReadHandle(); // 管线化请求
           else if(!error&& m_conn!= DISCONNCTED){
               event |=EPOLLIN;
               In_Buffer.clear();
           }
       }

    }
}

void HttpData::WriteHandle() {
    if(!error && m_conn != DISCONNCTED){
        uint32_t &event = m_Channel->GetEvent();
        int fd = m_Channel->Getfd();
        if(Util::Writen(fd,Out_Buffer)<0){ // write函数内部已经对其进行剪切
            error = true;
            return;
        }
        std::cout<<"Out_Buffer.size(): "<<Out_Buffer.size()<<std::endl;
        if(Out_Buffer.size()) event |= EPOLLOUT; // 如果没有写完的话，则将标志置为EPOLLOUT，等待触发
    }

}

void HttpData::ConnHandle() {
    SeparateTimer(); // 与定时器分离
    uint32_t &event = m_Channel->GetEvent();
    if(!error && m_conn == CONNECTED){
        int timeout = DEFAULT_EXPIRED_TIME;
        if(KeepAlive) timeout = DEFAULT_KEEPALIVE_TIME;
        if(event!=0){

            if((event&EPOLLIN)&&(event&EPOLLOUT)){
                event = 0;
                event = EPOLLOUT;
            }
            event |=EPOLLET;

        }else{
            event |=EPOLLIN|EPOLLET;
        }
        m_loop->UpdatePoller(m_Channel,timeout);

    }else if(!error&&m_conn==DISCONNECTING &&(event&EPOLLOUT)){ // 优雅关闭
        event = EPOLLOUT|EPOLLET;
        //m_loop->UpdatePoller(m_Channel,0);
    }
    else{
        m_loop->RunInLoop(std::bind(&HttpData::CloseHandle,this));
    }
}

void HttpData::ErrorHandle(int fd, int code, std::string msg) {
    msg = " "+msg;
    char SendBuff[4096];
    std::string BodyBuff,HeaderBuff;
    BodyBuff += "<html><title>wulala~出错啦～</title>";
    BodyBuff += "<body bgcolor=\"ffffff\">";
    BodyBuff += std::to_string(code)+msg;
    BodyBuff += "<hr><em>SimpleWebServer</em>\n</body></html>";

    HeaderBuff += "HTTP/1.1 "+std::to_string(code)+msg+"\r\n";
    HeaderBuff += "Content-Type:text/html\r\n";
    HeaderBuff += "Connection:Close";
    HeaderBuff += "Content_Length:"+std::to_string(BodyBuff.size())+"\r\n";
    HeaderBuff += "Server:SimpleWebServer\r\n";

    HeaderBuff += "\r\n";

    sprintf(SendBuff,"%s",HeaderBuff.c_str());
    sprintf(SendBuff,"%s",BodyBuff.c_str());
    Util::Writen(fd,SendBuff,strlen(SendBuff));
}

void HttpData::CloseHandle() {
    m_conn = DISCONNCTED;
    std::shared_ptr<HttpData> guard(shared_from_this()); // 可以从当前对象中取出指向该对象的shared_ptr指针

    m_Channel->SetConnCallBack(nullptr);
    m_Channel->SetReadCallBack(nullptr);
    m_Channel->SetWriteCallBack(nullptr);

    m_loop->RemovePoller(m_Channel);
}

LineState HttpData::ProcessRequestLine() {
    std::string& str = In_Buffer;
    int pos = str.find("\r\n");
    if(pos== std::string::npos){
        return LINE_ERROR;
    }
    // 去掉请求行所占空间，节省空间
    std::string RequestLine = str.substr(0,pos);
    if(str.size()>pos+2)
        str = str.substr(pos+2);
    else
        str.clear();
    // method
    int PosGet = RequestLine.find("GET");
    int PosHead = RequestLine.find("HEAD");
    int PosPost = RequestLine.find("POST");

    if(PosGet>=0){
        pos = PosGet;
        m_method = GET;
    }else if(PosHead>=0){
        pos = PosHead;
        m_method = HEAD;
    }else if(PosPost>=0){
        pos = PosPost;
        m_method = POST;
    } else
        return LINE_ERROR;

    // filename
    pos = RequestLine.find('/',pos);
    if(pos<0){
        std::cout<<"pos<0"<<std::endl;
        m_filename = "index.html";
        m_version = HTTP_11;
        return LINE_SUCCESS;
    }
    else{
        int blank  = RequestLine.find(' ',pos);
        if(blank == std::string::npos){
            return LINE_ERROR;
        }
        else{
            if(blank-pos>1){
                std::cout<<"blank-pos>1"<<std::endl;
                m_filename = RequestLine.substr(pos+1,blank-pos-1);
                size_t question = m_filename.find('?');
                if(question>=0){
                    m_filename = m_filename.substr(0,question);
                }
            } else
                m_filename = "index.html";
        }
        pos = blank;
    }

    std::cout<<"m_filename is "<<m_filename<<std::endl;

    // httpversion
    size_t httppos = RequestLine.find("HTTP/1.",pos);
    if(httppos== std::string::npos){
        return LINE_ERROR;
    } else{
        if(RequestLine.size()-httppos<=6){
            return LINE_ERROR;
        } else{
            char ver = RequestLine[httppos+7];
            if(ver=='1')
                m_version = HTTP_11;
            else if(ver=='0')
                m_version = HTTP_10;
            else
                return LINE_ERROR;
        }
    }

    return LINE_SUCCESS;
}

HeaderState HttpData::ProcessRequestHeader() {
    std::string &str = In_Buffer;
    int now_readlineBegin = 0;
    //std::cout<<"Begin ParseHeader,str is"<<str<<"\r\n";
    size_t pos = str.find("\r\n",now_readlineBegin);
    if(pos == std::string::npos) return HEADER_AGAIN;
    while(pos!=std::string::npos){
        if(pos<=now_readlineBegin) break;

        std::string request = str.substr(now_readlineBegin,pos-now_readlineBegin);
        //std::cout<<"request is "<< request <<std::endl;
        size_t colon = request.find(':');
        if(colon<0)
            return HEADER_ERROR;
        std::string key = str.substr(now_readlineBegin,colon);
        std::string value = str.substr(colon+1);
        headers[key] = value;
        now_readlineBegin = pos + 2;
        pos = str.find("\r\n",now_readlineBegin);
    }
    str = str.substr(now_readlineBegin+2); // 将头部信息从in_buffer中剔除
    //std::cout<<"ParseHeader,str :" <<str<<std::endl;
    return HEADER_SUCCESS;
}

ResponseState HttpData::ProcessResponse() {
    if(m_method == POST){
        // 暂不处理
    }else if(m_method == GET ||m_method == HEAD){
        std::string header;
        header += "HTTP/1.1 200 OK\r\n";
        if(headers.find("Connection")!=headers.end()&&(headers["Connection"]=="Keep-Alive"||headers["Connection"]=="keep-alive")){
            KeepAlive = true;
            header += std::string("Connection:Keep-Alive\r\n")+"Keep-Alive: timeout="+std::to_string(DEFAULT_KEEPALIVE_TIME)+"\r\n";
        }

        int dot_pos= m_filename.find('.');
        std::string filetype;
        if(dot_pos==std::string::npos){
            filetype = MimeType::getMime("default");
        }else{
            filetype = MimeType::getMime(m_filename.substr(dot_pos+1));
        }

        // echo test
        if(m_filename == "hello"){
            Out_Buffer = "HTTP/1.1 200 OK\r\nContent-type:text/plain\r\nContent-Length:11\r\n\r\nHello World";
            return RESPONSE_SUCCESS;
        }

        struct stat sbuf;
        if(stat(m_filename.c_str(),&sbuf)<0){
            header.clear();
            ErrorHandle(m_fd,404,"Not Found");
            return RESPONSE_ERROR;
        }

    }
}