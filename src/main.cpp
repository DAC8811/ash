#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <sys/timerfd.h>

#include <network/EventLoop.h>
#include <network/Channel.h>
#include <muduo/CurrentThread.h>
#include <muduo/Thread.h>


using namespace std;
using namespace ash;

network::EventLoop* g_loop;
void timeout(){
    printf("Timeout!\n");
    g_loop->quit();
}

int main(){
    network::EventLoop loop;
    g_loop = &loop;

    int timerfd = ::timerfd_create(CLOCK_MONOTONIC,TFD_NONBLOCK | TFD_CLOEXEC);
    network::Channel channel(&loop,timerfd);
    channel.setReadCallback(timeout);
    channel.enableReading();

    struct itimerspec howlong;
    bzero(&howlong,sizeof(howlong));
    howlong.it_value.tv_sec = 5;
    ::timerfd_settime(timerfd,0,&howlong,NULL);

    loop.loop();

    ::close(timerfd);
    

}