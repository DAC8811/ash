#pragma once

#include "muduo/noncopyable.h"
#include "muduo/Timestamp.h"
#include "network/EventLoop.h"


#include <vector>
#include <map>

namespace ash{
    namespace network{
        
        class Channel;

        ///EvenLoop的间接成员，只供其owner EvenLoop在IO线程调用
        ///自身并不拥有Channel
        ///仅负责IO复用查询而不负责事件分发
        class Poller:muduo::noncopyable
        {
         public:
            //typedef在类内部使用则限定了类型别名的作用域
            //public下的类型别名可以在外部通过类名::类型别名的方式使用
            typedef std::vector<Channel*> ChannelList;

            Poller(EventLoop* loop);
            ~Poller();
            ///Poller的核心，对poll函数的封装，当fd数组出现了活动事件，则将有活动事件的的fd描述符对应的channel对象传入activeChannels指向的channel数组中
            muduo::Timestamp poll(int timeoutMs,ChannelList* activeChannels);
            ///维护和更新pollfd_数组，其调用顺序为：
            ///用户在EventLoop中对channel进行设置->channel使用update->loop使用updateChannel->poller使用updateChannel
            void updateChannel(Channel* channel);

            void assertInLoopThread(){
                ownerLoop_->assertInLoopThread();
            }

         private:
            ///遍历pollfd数组，找出有活动事件的fd，将其对应Channel填入activeChannels
            void fillActiveChannels(int numEvents,ChannelList* activeChannels) const;

            typedef std::vector<struct pollfd> PollFdList;
            //private下的类型别名无法在外部使用
            typedef std::map<int,Channel*> ChannelMap;

            EventLoop* ownerLoop_;
            ///缓存pollfd数组
            PollFdList pollfds_;
            ///存储从fd到Channel*的映射关系(注意：Poller本身并不拥有Channel，Channel的实体存在于EventLoop中)
            ChannelMap channels_;
        };


    }
}