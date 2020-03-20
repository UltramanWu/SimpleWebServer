cmake_minimum_required(VERSION 3.10)
project(SimpleWebserver)

set(CMAKE_CXX_STANDARD 11)

add_executable(SimpleWebserver main.cpp
        base/noncopyable.h base/MutexLock.h
        Channel.cpp Channel.h
        EventLoop.cpp EventLoop.h
        Poller.cpp Poller.h base/Condition.h
        base/CurrentThread.h
        base/CountDownLatch.cpp base/CountDownLatch.h
        base/Thread.cpp base/Thread.h base/CurrentThread.cpp
        TimerQueue.cpp TimerQueue.h
        EventLoopThread.cpp EventLoopThread.h
        EventLoopThreadPool.cpp EventLoopThreadPool.h
        Util.h Util.cpp
        Server.cpp Server.h
        HttpData.cpp HttpData.h)


# thread不是本身自带的库，需要进行链接
find_package(Threads REQUIRED)
target_link_libraries(SimpleWebserver Threads::Threads)