#include "network/EventLoop.h"
#include "muduo/Logging.h"

using namespace ash;
using namespace ash::network;
///__thread关键字声明的变量必须为全局变量或函数内的静态变量，且每一个线程有一份独立实体，各个线程的值互不干扰
__thread EventLoop* t_loopInThisThread = 0;

EventLoop::EventLoop():
looping_(false),
threadId_(muduo::CurrentThread::tid())
{
    LOG_TRACE << "EventLoop created " << this << " in thread " << threadId_;
    if(t_loopInThisThread){
        LOG_FATAL << "Another EventLoop " << t_loopInThisThread << "exists in this thread " << threadId_;
    }
    else{
        t_loopInThisThread = this;
    }
}

EventLoop::~EventLoop(){
    //assert断言，若传入参数为假，则马上退出程序
    assert(!looping_);
    t_loopInThisThread = NULL;
}

inline void EventLoop::assertInLoopThread(){
    if(!isInLoopThread())
        abortNotInLoopThread();
}

inline bool EventLoop::isInLoopThread() const{
    return threadId_ == muduo::CurrentThread::tid();
}

EventLoop* EventLoop::getEventLoopOfCurrentThread(){
    return t_loopInThisThread;
}

void EventLoop::abortNotInLoopThread(){
      LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
            << " was created in threadId_ = " << threadId_
            << ", current thread id = " <<  muduo::CurrentThread::tid();
}

void EventLoop::loop(){
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;

    //::poll(NULL,0,5*1000);
    sleep(5);

    LOG_TRACE << "EventLoop " << this << " stop looping";
    looping_ = false;
}