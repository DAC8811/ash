#include "muduo/noncopyable.h"
#include <atomic>
#include "muduo/CurrentThread.h"
namespace ash{
    namespace network{
        ///EventLoop，即事件循环，占用一个线程进行具体的IO处理，只能在被创建的线程调用
        class EventLoop : muduo::noncopyable{
        public:
            ///初始化loop，检查线程中是否已有其他loop
            EventLoop();
            ~EventLoop();
            void loop();
            ///检查当前线程是否是对象创建时的线程
            void assertInLoopThread();
            ///查看当前线程是否是loop被创建的线程
            bool isInLoopThread() const;
            ///返回指向当前线程中的唯一loop对象的指针
            static EventLoop* getEventLoopOfCurrentThread();
        private:
            void abortNotInLoopThread();
            bool looping_;
            ///pid_t是linux中用于表示进程类型的一种类型
            const pid_t threadId_;
        };
    }
}
