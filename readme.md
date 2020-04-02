SimpleWebServer:C++11 实现的静态Http服务器，支持GET、HEAD和http长连接，支持管线化请求。

**technology Point**:
* 1. 使用Reactor模式+epoll边沿触发+非阻塞IO的组合形式；
* 2. 使用线程池，减少线程频繁创建与销毁的开销；
* 3. 使用timerfd+红黑树的定时器形式，处理超时请求；
* 4. 使用evenfd实现线程的异步唤醒；
* 5. 主线程只负责accept请求，并以Round Robin的方式将分发给其他线程，锁的竞争只出现在主线程与指定分发的线程之间，减少race condition的发生；
* 6. 支持优雅的关闭连接；
* 7. 使用智能指针管理对象，减少内存泄露。
