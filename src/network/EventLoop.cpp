#include <algorithm>

#include "network/EventLoop.h"
#include "network/Poller.h"
#include "network/Channel.h"
#include "network/TimerId.h"
#include "network/TimerQueue.h"
#include "muduo/Logging.h"



using namespace ash;
using namespace ash::network;

const int kPollTimeMs = 10000;

///__thread关键字声明的变量必须为全局变量或函数内的静态变量，且每一个线程有一份独立实体，各个线程的值互不干扰
__thread EventLoop* t_loopInThisThread = 0;

EventLoop::EventLoop():
looping_(false),
threadId_(muduo::CurrentThread::tid()),
poller_(new Poller(this)),
timerQueue_(new TimerQueue(this))
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
    quit_ = false;

    while(!quit_){
        activeChannels_.clear();
        //调用Poller::poll()函数获取当前活动事件的channel列表，然后依次调用每个channel的handleEvent()函数
        pollReturnTime_ = poller_->poll(kPollTimeMs,&activeChannels_);
        for(ChannelList::iterator it = activeChannels_.begin(); it != activeChannels_.end(); it++){
            (*it)->handleEvent(pollReturnTime_);
        }
    }

    LOG_TRACE << "EventLoop " << this << " stop looping";
    looping_ = false;
}

void EventLoop::updateChannel(Channel* channel){
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    poller_->updateChannel(channel);
}

void EventLoop::quit(){
    quit_ = true;
}

void EventLoop::queueInLoop(const Functor& cb){
    {
        muduo::MutexLockGuard lock(mutex_);
        pendingFunctors_.push_back(cb);
    }

    if(!isInLoopThread() || callingPendingFunctors_){
        wakeup();
    }
}

void EventLoop::runInLoop(const Functor& cb){
    if(isInLoopThread()){
        cb();
    }
    else{
        queueInLoop(cb);
    }
}

TimerId EventLoop::runAt(const muduo::Timestamp& time,const TimerCallback& cb){
    return timerQueue_->addTimer(cb,time,0.0);
}
TimerId EventLoop::runAfter(double delay,const TimerCallback& cb){
    muduo::Timestamp time(addTime(muduo::Timestamp::now(),delay));
    return runAt(time,cb);
}
TimerId EventLoop::runEvery(double interval,const TimerCallback& cb){
    muduo::Timestamp time(addTime(muduo::Timestamp::now(),interval));
    return timerQueue_->addTimer(cb,time,interval);
}

void EventLoop::wakeup()
{
  uint64_t one = 1;
//  ssize_t n = sockets::write(wakeupFd_, &one, sizeof one);
//   if (n != sizeof one)
//   {
//     LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
//   }
}

void EventLoop::removeChannel(Channel* channel)
{
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  if (eventHandling_)
  {
    assert(currentActiveChannel_ == channel ||
        std::find(activeChannels_.begin(), activeChannels_.end(), channel) == activeChannels_.end());
  }
  poller_->removeChannel(channel);
}