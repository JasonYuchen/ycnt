//
// Created by jason on 2019/10/2.
//

#include <ycnt/base/Thread.h>
#include <ycnt/net/EventLoop.h>

using namespace ycnt;
using namespace ycnt::net;

__thread EventLoop *t_eventLoop = nullptr;

EventLoop::EventLoop()
{

}

EventLoop::~EventLoop()
{

}