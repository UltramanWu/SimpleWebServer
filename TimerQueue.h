//
// Created by wu on 3/18/20.
//

#ifndef SIMPLEWEBSERVER_TIMERQUEUE_H
#define SIMPLEWEBSERVER_TIMERQUEUE_H

#include <memory>
#include <sys/time.h>
#include <queue>

class HttpData;

typedef std::shared_ptr<HttpData> SPHttpData;

class TimerNode{
public:
    TimerNode(SPHttpData httpdata,int timeout);
    ~TimerNode();

    void Update(int timeout){
        struct timeval now;
        gettimeofday(&now,NULL); // 时间结构体 时区
        expiredTime = (((now.tv_sec%10000)*1000)+(now.tv_usec/1000))+timeout;
    }

    void SetExpiredTime(int timeout){
        struct timeval now;
        gettimeofday(&now,NULL); // 时间结构体 时区
        expiredTime = (((now.tv_sec%10000)*1000)+(now.tv_usec/1000))+timeout;
    }

    void SetHttpData(SPHttpData httpData){
        m_httpdata = httpData;
    }

    bool IsValid(){
        struct timeval now;
        gettimeofday(&now,NULL); // 时间结构体 时区
        size_t tmp = (((now.tv_sec%10000)*1000)+(now.tv_usec/1000));
        if(tmp < expiredTime){
            return true;
        }else{
            this->SetDeleted();
            return false;
        }
    }

    bool GetDeleted(){
        return deleted;
    }

    void SetDeleted(){
        deleted = true;
    }
    void ResetDeleted(){
        deleted = false;
    }

    void ClearReq(){
        // HttpData指针释放
        m_httpdata.reset();
        SetDeleted();
    }

    size_t ExpiredTime(){ return expiredTime; }
private:
    SPHttpData m_httpdata;
    size_t expiredTime;
    bool deleted;
};

typedef std::shared_ptr<TimerNode> SPTimerNode;

struct TimerCmp{
    bool operator()(std::shared_ptr<TimerNode> lhs,std::shared_ptr<TimerNode> rhs){
        return lhs->ExpiredTime()>rhs->ExpiredTime();
    }
};

class TimerQueue {
public:
    TimerQueue(){}
    ~TimerQueue(){}

    void AddTimer(SPHttpData httpData,int timeout); // 向定时器队列中插入事件
    void HandleExpired(); // 处理超时事件

private:
    std::queue<SPTimerNode> DelTimerNodes; // 用于存放从最小堆中剔除的时间节点，用于未来插入时间节点时使用，省去new和delete的开销
    std::priority_queue<SPTimerNode,std::deque<SPTimerNode>,TimerCmp> TimerNodeQueue;

};
#endif //SIMPLEWEBSERVER_TIMERQUEUE_H
