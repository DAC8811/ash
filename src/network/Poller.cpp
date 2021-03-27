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
        /**************更新的过程也就是将channel中携带的信息写入对应的pollfd结构体中*********/
        pfd.fd = channel->fd();
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        /****************************************************************************/
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
        if(channel->isNoneEvent())//如果该channel对任何活动都不关心，则直接将对象的fd其置为负数，这样poll就不可能收到其fd的任何信息
            pfd.fd = -channel->fd()-1;
    }
    

    
}

void Poller::removeChannel(Channel* channel)
{
  Poller::assertInLoopThread();
  LOG_TRACE << "fd = " << channel->fd();
  assert(channels_.find(channel->fd()) != channels_.end());
  assert(channels_[channel->fd()] == channel);
  assert(channel->isNoneEvent());
  int idx = channel->index();
  assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
  const struct pollfd& pfd = pollfds_[idx]; (void)pfd;
  assert(pfd.fd == -channel->fd()-1 && pfd.events == channel->events());
  //首先从映射表中删除fd->channel键值对，同时获取删除被删除的元素个数用于断言检查
  size_t n = channels_.erase(channel->fd());
  assert(n == 1); (void)n;
  //如果该fd本身就是最后一个，那么直接pop即可
  if (muduo::implicit_cast<size_t>(idx) == pollfds_.size()-1)
  {
    pollfds_.pop_back();
  }
  else
  {
    //获取队列最后一个pollfd的文件描述符
    int channelAtEnd = pollfds_.back().fd;
    //将要删除的pollfd与与队列最后一个pollfd进行替换
    iter_swap(pollfds_.begin()+idx, pollfds_.end()-1);
    if (channelAtEnd < 0)//如果此时fd为负数，则将其还原为其channel对应的fd
    {
      channelAtEnd = -channelAtEnd-1;
    }
    channels_[channelAtEnd]->set_index(idx);//重新设置换到中间位置的fd对应的channel所记录的index值
    pollfds_.pop_back();//删除最后一个pollfd
  }
}