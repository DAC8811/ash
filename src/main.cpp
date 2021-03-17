#include <iostream>
#include <stdio.h>
#include <network/EventLoop.h>
#include <muduo/CurrentThread.h>
#include <muduo/Thread.h>
#include <unistd.h>

using namespace std;
using namespace ash;

network::EventLoop* g_loop;
void threadFunc(){
    // printf("threadFunc(): pid = %d,tid = %d\n",getpid(),muduo::CurrentThread::tid());
    // ash::network::EventLoop loop;
    // loop.loop();
    g_loop->loop();
}

int main(){
    // printf("main(): pid = %d,tid = %d\n",getpid(),muduo::CurrentThread::tid());
    // ash::network::EventLoop loop;
    // muduo::Thread thread(threadFunc);
    // thread.start();


    // loop.loop();
    // pthread_exit(NULL);
    network::EventLoop loop;
    g_loop = &loop;
    muduo::Thread t(threadFunc);
    t.start();
    t.join();
}