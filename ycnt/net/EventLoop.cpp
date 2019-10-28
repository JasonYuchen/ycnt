//
// Created by jason on 2019/10/2.
//

#include <unistd.h>
#include <sys/eventfd.h>

#include <ycnt/base/Thread.h>
#include <ycnt/base/LogStream.h>
#include <ycnt/net/EventLoop.h>
#include <ycnt/net/poller/Poller.h>
#include <ycnt/net/Channel.h>

using namespace std;

namespace ycnt
{

namespace net
{

__thread EventLoop *t_eventLoop = nullptr;

constexpr int kPollTimeMs = 10000;

int createEventFd()
{
  int eventFd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (eventFd < 0) {
    LOG_FATAL << "::eventfd error: " << ::strerror(errno);
  }
  return eventFd;
}

EventLoop::EventLoop()
  : threadId_(currentThread::tid()),
    looping_(false),
    quit_(false),
    eventHandling_(false),
    functorHandling_(false),
    iteration_(0),
    poller_(Poller::newEpollPoller(this)),
    wakeupFd_(createEventFd()),
    wakeupChannel_(new Channel(this, wakeupFd_)),
    currentActiveChannel_(nullptr),
    maxQueueSize_(1000000)
{
  LOG_DEBUG << "EventLoop created in thread " << threadId_;
  if (t_eventLoop) {
    LOG_FATAL << "Another EventLoop exists in this thread, exiting...";
  } else {
    t_eventLoop = this;
  }
  wakeupChannel_->setReadCallback(
    [this](base::Timestamp)
    {
      int64_t v = 0;
      if (::read(wakeupFd_, &v, sizeof(v)) != sizeof(v)) {
        LOG_ERROR << "::read from wakeupFd error: " << ::strerror(errno);
      }
    });
  wakeupChannel_->enableReading();
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

void EventLoop::setContext(const any &context)
{
  context_ = context;
}

std::any &EventLoop::getContext()
{
  return context_;
}

const std::any &EventLoop::getContext() const
{
  return context_;
}

void EventLoop::runInLoop(Functor cb)
{
  if (isInLoopThread()) {
    cb();
  } else {
    queueInLoop(std::move(cb));
  }
}

void EventLoop::queueInLoop(Functor cb)
{
  {
    lock_guard<mutex> guard(mutex_);
    functors_.emplace_back(std::move(cb));
  }
  if (!isInLoopThread() || functorHandling_) {
    wakeup();
  }
}

size_t EventLoop::queueSize() const
{
  lock_guard<mutex> guard(mutex_);
  return functors_.size();
}

void EventLoop::wakeup()
{
  int64_t v = 0;
  if (::write(wakeupFd_, &v, sizeof(v)) != sizeof(v)) {
    LOG_ERROR << "::write to wakeupFd error: " << ::strerror(errno);
  }
}

void EventLoop::updateChannel(Channel *channel)
{
  assertInLoopThread();
  assert(channel->ownerLoop() == this);
  poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel)
{
  assertInLoopThread();
  assert(channel->ownerLoop() == this);
  if (eventHandling_) {
    assert(currentActiveChannel_ == channel ||
      std::find(activeChannels_.begin(), activeChannels_.end(), channel)
        == activeChannels_.end());
  }
  poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel *channel)
{
  assertInLoopThread();
  assert(channel->ownerLoop() == this);
  return poller_->hasChannel(channel);
}

void EventLoop::assertInLoopThread()
{
  if (!isInLoopThread()) {
    LOG_FATAL << "EventLoop was created in thread " << threadId_
              << ", current thread " << currentThread::tid()
              << ", aborting...";
  }
}

bool EventLoop::isInLoopThread()
{
  return threadId_ == currentThread::tid();
}

} // namespace net

} // namespace ycnt

