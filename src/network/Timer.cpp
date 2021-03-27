
#include "network/Timer.h"

using namespace ash;
using namespace ash::network;

muduo::AtomicInt64 Timer::s_numCreated_;

void Timer::restart(muduo::Timestamp now)
{
  if (repeat_)
  {
    expiration_ = addTime(now, interval_);
  }
  else
  {
    expiration_ = muduo::Timestamp::invalid();
  }
}
