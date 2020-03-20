//
// Created by wu on 3/18/20.
//

#include "Thread.h"
#include <cassert>

Thread::Thread(const Functor &functor):ThreadFunc(functor),IsStarted(false),IsJoined(false) {}

Thread::~Thread() {
    if(IsStarted&&!IsJoined)
        m_thread->detach();
}

void Thread::start() {
    assert(!IsStarted);
    assert(!m_thread);
    m_thread.reset(new std::thread(ThreadFunc));
    IsStarted = true;
}



void Thread::join() {
    assert(IsStarted);
    assert(!IsJoined);
    m_thread->join();
    IsJoined = true;
}

