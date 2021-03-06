#pragma once

#include "muduo/noncopyable.h"
#include "muduo/CurrentThread.h"
#include "muduo/Timestamp.h"
#include "network/Callbacks.h"
#include "muduo/Mutex.h"


#include <vector>
#include <atomic>
#include <memory>


namespace ash{
    namespace network{

        class Channel;
        class Poller;
        class TimerId;
        class TimerQueue;

        ///EventLoop，即事件循环，占用一个线程进行具体的IO处理，只能在被创建的线程调用
        class EventLoop : muduo::noncopyable{
         public:
            typedef std::function<void()> Functor;
            ///初始化loop，检查线程中是否已有其他loop
            EventLoop();
            ~EventLoop();
            void loop();
            void quit();
            ///检查当前线程是否是对象创建时的线程
            void assertInLoopThread(){
                if(!isInLoopThread())
                    abortNotInLoopThread();
            }
            ///查看当前线程是否是loop被创建的线程
            bool isInLoopThread() const{
                return threadId_ == muduo::CurrentThread::tid();
            }
            ///返回指向当前线程中的唯一loop对象的指针
            static EventLoop* getEventLoopOfCurrentThread();

            void wakeup();
            ///根据某一个channel对pollfd进行更新，实际上是通过poller对其包含的fd进行修改
            void updateChannel(Channel* channel);
            void removeChannel(Channel* channel);

            void runInLoop(const Functor& cb);

            void queueInLoop(const Functor& cb);

            TimerId runAt(const muduo::Timestamp& time,const TimerCallback& cb);
            TimerId runAfter(double delay,const TimerCallback& cb);
            TimerId runEvery(double interval,const TimerCallback& cb);

            
         private:
            typedef std::vector<Channel*> ChannelList;

            muduo::Timestamp pollReturnTime_;

            void abortNotInLoopThread();

         

            bool looping_;
            ///可通过设置其来终止loop循环
            bool quit_;
            ///pid_t是linux中用于表示进程类型的一种类型
            const pid_t threadId_;
            bool eventHandling_; /* atomic */
            bool callingPendingFunctors_;
            ///unique_ptr相当于原始指针，在其超出作用域时会自动析构，无需手动释放，无法被复制
            //指向其对应的一个poller
            std::unique_ptr<Poller> poller_;
            ///指向其对应的一个timerQueue，所有的定时操作都可以由该timerQueue完成
            std::unique_ptr<TimerQueue> timerQueue_;
            ///存储有活动事件的channel的channel数组
            ChannelList activeChannels_;
            Channel* currentActiveChannel_;

            mutable muduo::MutexLock mutex_;
            std::vector<Functor> pendingFunctors_ GUARDED_BY(mutex_);
        };
    }
}
