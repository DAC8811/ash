#include "network/EventLoop.h"
#include "muduo/Logging.h"

using namespace ash;
using namespace ash::network;

__thread EventLoop* t_loopInThisThread = 0;

EventLoop::EventLoop():
looping_(false),
threadId_(muduo::CurrentThread::tid())
{
    LOG_TRACE << "EventLoop created" << this << "in thread" << threadId_;
}