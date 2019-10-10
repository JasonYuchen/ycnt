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

constexpr int kPollTimeMs = 10000;

EventLoop::EventLoop()
  : threadId_(currentThread::tid()),
    looping_(false),
    quit_(false),
    eventHandling_(false),
    functorHandling_(false),
    iteration_(0),
    poller_(Poller::newEpollPoller(this))
{
  LOG_DEBUG << "EventLoop created in thread " << threadId_;
  if (t_eventLoop) {
    LOG_FATAL << "Another EventLoop exists in this thread, exiting...";
  } else {
    t_eventLoop = this;
  }
  // wakeup
}

EventLoop::~EventLoop()
{
  LOG_DEBUG << "EventLoop of thread " << threadId_
            << " destroyed in thread " << currentThread::tid();
  // close wakeup
  t_eventLoop = nullptr;
}

void EventLoop::loop()
{
  assert(!looping_);
  assertInLoopThread();
  looping_ = true;
  quit_ = false;
  LOG_TRACE << "EventLoop start looping in thread " << currentThread::tid();
  while (quit_) {
    pollReturnTime_ = poller_->poll(kPollTimeMs, activeChannels_);
    ++iteration_;
  }
  LOG_TRACE << "EventLoop stop looping in thread " << currentThread::tid();
  looping_ = false;
}

void EventLoop::quit()
{
  quit_ = true;
  if (threadId_ != currentThread::tid()) {
    // wake up the eventloop
  }
}

int64_t EventLoop::iteration() const
{
  return iteration_;
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

