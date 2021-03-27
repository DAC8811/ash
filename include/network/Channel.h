#pragma once

#include "muduo/noncopyable.h"
#include "muduo/Timestamp.h"

#include <functional>


namespace ash{
    namespace network{
        //预先声明EventLoop类，这样就不需要在头文件中包含EventLoop.h
        class EventLoop;

        ///Channel：对象负责一个具体的文件描述符(fd)的IO事件分发，本质上就是对pollfd的一层封装，channel中含有其对应pollfd的三个基本属性:fd,revents,events
        ///但它并不含有该fd，在析构时也不会关闭该fd
        ///Channel会把不同的IO事件分发给不同的回调，回调则使用std::function进行具体的回调函数注册
        ///每个Channel对象自始至终只属于一个EvenLoop，也即只属于一个线程
        class Channel : muduo::noncopyable{
        public:
            typedef std::function<void()> EventCallback;
            ///在channel初始化时，需要告知拥有其的loop以及其对应的文件描述符fd(一般在网络应用中就对应一个socket连接)
            Channel(EventLoop* loop, int fd);
            ~Channel();

            ///Channel的核心函数，由loop函数调用
            ///根据revents_的值分别调用不同的用户回调
            void handleEvent(muduo::Timestamp receiveTime);
            void setReadCallback(EventCallback cb)
            { readCallback_ = std::move(cb); }//右值引用，移动构造后cb将为空，而readCallback_将拥有资源，减少了cb拷贝到readCallback_的一次拷贝过程
            void setWriteCallback(EventCallback cb)
            { writeCallback_ = std::move(cb); }
            void setErrorCallback(EventCallback cb)
            { errorCallback_ = std::move(cb); }

            int fd() const { return fd_; }
            int events() const { return events_; }
            void set_revents(int revt) { revents_ = revt; } // used by pollers
            // int revents() const { return revents_; }
            ///将channel设置为不关心任何事件
            bool isNoneEvent() const { return events_ == kNoneEvent; }

            void enableReading() { events_ |= kReadEvent; update(); }
            void disableReading() { events_ &= ~kReadEvent; update(); }
            void enableWriting() { events_ |= kWriteEvent; update(); }
            void disableWriting() { events_ &= ~kWriteEvent; update(); }
            void disableAll() { events_ = kNoneEvent; update(); }
            bool isWriting() const { return events_ & kWriteEvent; }
            bool isReading() const { return events_ & kReadEvent; }

            // for Poller
            int index() { return index_; }
            void set_index(int idx) { index_ = idx; }

            EventLoop* ownerLoop() { return loop_; }
            void remove();
        private:
            ///update的这个操作只有在修改了关心事件后才会调用，本质上就是通知自己的主loop对自己使用updateChannel函数
            void update();

            static const int kNoneEvent;
            static const int kReadEvent;
            static const int kWriteEvent;

            EventLoop* loop_;
            //poll中每一个连接通道对应的文件描述符,该fd在channel构造时被设置，之后就无法改变了
            const int  fd_;
            ///该channel所关心的具体的IO事件，由创建者设置
            int        events_;
            ///目前活跃的事件，由EventLoop/Poller设置
            int        revents_;
            /// used by Poller.本质上就是channel中包含的文件描述符fd在poller存储的fd数组中的下标位置
            int        index_; 

            bool addedToLoop_;

            EventCallback writeCallback_;
            EventCallback readCallback_;
            EventCallback errorCallback_;


        };
    }
}
