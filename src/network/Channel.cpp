#include "network/Channel.h"
#include "network/EventLoop.h"
#include "muduo/Logging.h"

#include <poll.h>

using namespace ash;
using namespace network;

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop, int fd)
  : loop_(loop),
    fd_(fd),
    events_(0),
    revents_(0),
    index_(-1)
{
}

Channel::~Channel()
{
}

void Channel::update(){
    addedToLoop_ = true;
    loop_->updateChannel(this);
}

void Channel::handleEvent(muduo::Timestamp receiveTime){
    if(revents_ & POLLNVAL){
        LOG_WARN << "Channel::handle_event() POLLNYAL";
    }

    if(revents_ & (POLLERR | POLLNVAL)){
        if(errorCallback_)
            errorCallback_();
    }

    if(revents_ & (POLLIN | POLLPRI | POLLRDHUP)){
        if(readCallback_)
            readCallback_();
    }

    if(revents_ & POLLOUT){
        if(writeCallback_)
            writeCallback_();
    }

}

void Channel::remove()
{
  assert(isNoneEvent());
  addedToLoop_ = false;
  loop_->removeChannel(this);
}