//
// Created by wu on 3/18/20.
//

#include "TimerQueue.h"
#include "HttpData.h"


TimerNode::TimerNode(SPHttpData httpdata,int timeout):m_httpdata(httpdata),deleted(false){
    struct timeval now;
    gettimeofday(&now,NULL); // 时间结构体 时区
    expiredTime = (((now.tv_sec%10000)*1000)+(now.tv_usec/1000))+timeout;
}

TimerNode::~TimerNode() {
    if(m_httpdata) m_httpdata->CloseHandle();

}

void TimerQueue::AddTimer(SPHttpData httpData, int timeout) {
    SPTimerNode item(std::make_shared<TimerNode>(httpData,timeout));
    TimerNodeQueue.push(item);
    httpData->LinkTimer(item); // 将TimerNode与HttpData进行关联
}


void TimerQueue::HandleExpired() {
    while(!TimerNodeQueue.empty()){
        SPTimerNode tn = TimerNodeQueue.top();
        if(tn->IsValid()==false || tn->GetDeleted()==true){
            TimerNodeQueue.pop();
        }
        else
            break;
    }
}