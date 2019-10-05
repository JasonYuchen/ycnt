//
// Created by jason on 2019/10/2.
//

#include <ycnt/base/Thread.h>
#include <ycnt/base/LogStream.h>
#include <ycnt/net/EventLoop.h>
#include <ycnt/net/poller/Poller.h>

namespace ycnt
{

namespace net
{

__thread EventLoop *t_eventLoop = nullptr;

EventLoop::EventLoop()
  : threadId_(currentThread::tid()),
    looping_(false),
    quit_(false),
    eventHandling_(false),
    functorHandling_(false),
    iteration_(0),
    poller_(Poller::newEpollPoller(this))
{
  // LOG
}

EventLoop::~EventLoop()
{

}

void EventLoop::loop()
{

}

void EventLoop::assertInLoopThread()
{
  if (threadId_ != currentThread::tid()) {
    LOG_FATAL << "EventLoop was created in thread " << threadId_
              << ", current thread " << currentThread::tid()
              << ", aborting...";
  }
}

} // namespace net

} // namespace ycnt

