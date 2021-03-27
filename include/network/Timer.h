#pragma once

#include "muduo/noncopyable.h"
#include "muduo/Atomic.h"
#include "network/Callbacks.h"

namespace ash{
    namespace network{
        class Timer : muduo::noncopyable
        {
        public:
        ///该类并不会被用户直接调用
        ///在使用上模拟一个定时器，但实际上并不含有一个定时器文件，多个timer共享一个定时器文件
        Timer(TimerCallback cb, muduo::Timestamp when, double interval)
         :  callback_(std::move(cb)),
            expiration_(when),
            interval_(interval),
            repeat_(interval > 0.0),
            sequence_(s_numCreated_.incrementAndGet())
        { }

        void run() const
        {
            callback_();
        }

        ///返回过期时间
        muduo::Timestamp expiration() const  { return expiration_; }
        bool repeat() const { return repeat_; }
        int64_t sequence() const { return sequence_; }

        void restart(muduo::Timestamp now);

        static int64_t numCreated() { return s_numCreated_.get(); }

        private:
        ///响应函数
        const TimerCallback callback_;
        ///过期时间
        muduo::Timestamp expiration_;
        ///定时时间间隔(设为0则是一次性定时器)
        const double interval_;
        ///是否是一次性的定时器
        const bool repeat_;
        const int64_t sequence_;

        static muduo::AtomicInt64 s_numCreated_;
        };
    }
}
