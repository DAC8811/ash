#pragma once

#include <set>
#include <vector>

#include "muduo/noncopyable.h"
#include "muduo/Timestamp.h"
#include "network/Callbacks.h"
#include "network/Channel.h"

namespace ash{
    namespace network{
        class EventLoop;
        class TimerId;
        class Timer;

        ///用于用户使用的定时器接口
        ///本质上是使用一个定时器实现定时队列的功能
        class TimerQueue : muduo::noncopyable{
         public:
            TimerQueue(EventLoop* loop);
            ~TimerQueue();
            ///必须是线程安全的，因为经常被其他线程调用
            TimerId addTimer(const TimerCallback& cb,muduo::Timestamp,double interval);
            void cancel(TimerId timerId);
         private:
            ///使用pair作为键值，主要防止到期时间相同的timer删除时发生冲突
            typedef std::pair<muduo::Timestamp,Timer*> Entry;
            ///使用pair作为存储对象时，set默认首先比较pair的first元素，如果first元素相同则比较其second元素
            typedef std::set<Entry> TimerList;
            typedef std::pair<Timer*, int64_t> ActiveTimer;
            typedef std::set<ActiveTimer> ActiveTimerSet;

            void cancelInLoop(TimerId timerId);
            void addTimerInLoop(Timer* timer);
            ///当timerfd有活动事件时被唤起
            void handleRead();
            ///移除所有的过期timer
            std::vector<Entry> getExpired(muduo::Timestamp now);
            void reset(const std::vector<Entry>& expired,muduo::Timestamp now);

            bool insert(Timer* timer);

            

            EventLoop* loop_;
            ///timer对应的文件描述符，一个timrQueue对象只有一个，无法更改
            const int timerfd_;
            ///用于读取timer的channel
            Channel timerfdChannel_;
            ///根据到期时间排序的timer列表
            TimerList timers_;

            ActiveTimerSet activeTimers_;
            bool callingExpiredTimers_; 
            ActiveTimerSet cancelingTimers_;
        };
    }
}
