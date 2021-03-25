#include <poll.h>

#include "network/Poller.h"
#include "muduo/Logging.h"
#include "network/Channel.h"





using namespace ash;
using namespace ash::network;

Poller::Poller(EventLoop* loop)
  : ownerLoop_(loop)
{
}

Poller::~Poller()
{
}

muduo::Timestamp Poller::poll(int timeoutMs,ChannelList* activeChannels){
    //使用标准库的poll函数进行事件等待
    int numEvents = ::poll(&*pollfds_.begin(),pollfds_.size(),timeoutMs);
    muduo::Timestamp now(muduo::Timestamp::now());
    if(numEvents > 0){
        LOG_TRACE << numEvents << " events happended";
        fillActiveChannels(numEvents,activeChannels);
    }
    else if(numEvents == 0)
        LOG_TRACE << "nothing happended";
    else    
        LOG_SYSERR << "Plloer::poll()";
    return now;
}

void Poller::fillActiveChannels(int numEvents,ChannelList* activeChannels) const
{
    for(PollFdList::const_iterator pfd = pollfds_.begin(); pfd != pollfds_.end() && numEvents > 0; pfd++){
        if(pfd->revents > 0){
            numEvents--;
            ChannelMap::const_iterator ch = channels_.find(pfd->fd);
            assert(ch != channels_.end());
            Channel* channel = ch->second;
            channel->set_revents(pfd->revents);
            activeChannels->push_back(channel);
        }
    }
}

void Poller::updateChannel(Channel* channel){
    assertInLoopThread();
    LOG_TRACE << "fd = " << channel->fd() << " events = " << channel->events();
    if(channel->index() < 0){//说明是新建的channel，需要添加到pollfds_数组中
        assert(channels_.find(channel->fd()) == channels_.end());
        struct pollfd pfd;
        pfd.fd = channel->fd();
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        pollfds_.push_back(pfd);
        int idx = static_cast<int>(pollfds_.size())-1;
        channel->set_index(idx);
        channels_[pfd.fd] = channel;
    }
    else{//该channel已经存在，对其进行更新
        assert(channels_.find(channel->fd()) != channels_.end());
        assert(channels_[channel->fd()] == channel);
        int idx = channel->index();
        assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
        struct pollfd& pfd = pollfds_[idx];//这样能够直接传引用
        assert(pfd.fd == channel->fd() || pfd.fd == -1);
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        if(channel->isNoneEvent())
            pfd.fd = -1;
    }
    

    
}