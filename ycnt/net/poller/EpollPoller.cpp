//
// Created by jason on 2019/9/27.
//

#include <sys/epoll.h>
#include <unistd.h>

#include <ycnt/net/EventLoop.h>
#include <ycnt/net/poller/Poller.h>
#include <ycnt/net/poller/EpollPoller.h>
#include <ycnt/base/LogStream.h>
#include <ycnt/net/Channel.h>

namespace ycnt
{

namespace net
{

EpollPoller::EpollPoller(EventLoop *loop)
  : loop_(loop),
    epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
    events_(kInitEventsSize)
{
  if (epollfd_ < 0) {
    LOG_FATAL << "::epoll_create1 error: " << ::strerror(errno);
  }
}

EpollPoller::~EpollPoller()
{
  ::close(epollfd_);
}

base::Timestamp EpollPoller::poll(
  int timeoutMs,
  ChannelVec &activeChannels)
{
  int numEvents =
    ::epoll_wait(epollfd_, events_.data(), events_.size(), timeoutMs);
  auto now = base::Timestamp::now();
  if (numEvents > 0) {
    for (int i = 0; i < numEvents; ++i) {
      auto channel = static_cast<Channel *>(events_[i].data.ptr);
      assert(channels_.find(channel->fd()) != channels_.end());
      assert(channels_.find(channel->fd())->second == channel);
      channel->setRevents(events_[i].events);
      activeChannels.push_back(channel);
    }
    if (UNLIKELY(static_cast<size_t>(numEvents) == events_.size())) {
      events_.resize(events_.size() * 2);
    }
  } else if (numEvents == 0) {
    // nothing happened
  } else {
    LOG_ERROR << "epoll_wait error: " << ::strerror(errno);
  }
  return now;
}

void EpollPoller::updateChannel(Channel *channel)
{
  loop_->assertInLoopThread();
  LOG_TRACE << channel;
  Channel::Flag flag = channel->flag();
  int fd = channel->fd();
  switch (flag) {
    case Channel::Flag::kNew: {
      assert(channels_.find(fd) == channels_.end());
      assert(!channel->isNoneEvent());
      channels_[fd] = channel;
      channel->setFlag(Channel::Flag::kAdded);
      updateInEpoll(EPOLL_CTL_ADD, channel);
      break;
    }
    case Channel::Flag::kDeleted: {
      assert(channels_.find(fd) != channels_.end());
      assert(channels_[fd] == channel);
      assert(!channel->isNoneEvent());
      channel->setFlag(Channel::Flag::kAdded);
      updateInEpoll(EPOLL_CTL_ADD, channel);
      break;
    }
    case Channel::Flag::kAdded: {
      assert(channels_.find(fd) != channels_.end());
      assert(channels_[fd] == channel);
      if (channel->isNoneEvent()) {
        updateInEpoll(EPOLL_CTL_DEL, channel);
        channel->setFlag(Channel::Flag::kDeleted);
      } else {
        updateInEpoll(EPOLL_CTL_MOD, channel);
      }
      break;
    }
    default: {
      LOG_ERROR << "unexpected channel flag for fd = " << channel->fd();
    }
  }
}

void EpollPoller::removeChannel(Channel *channel)
{
  loop_->assertInLoopThread();
  LOG_TRACE << channel;
  Channel::Flag flag = channel->flag();
  int fd = channel->fd();
  assert(channels_.find(fd) != channels_.end());
  assert(channels_[fd] == channel);
  assert(channel->isNoneEvent());
  assert(flag != Channel::Flag::kNew);
  channels_.erase(fd);
  if (flag == Channel::Flag::kAdded) {
    updateInEpoll(EPOLL_CTL_DEL, channel);
  }
  channel->setFlag(Channel::Flag::kNew);
}

bool EpollPoller::hasChannel(Channel *channel) const
{
  loop_->assertInLoopThread();
  return channels_.find(channel->fd()) != channels_.end();
}

void EpollPoller::updateInEpoll(int op, Channel *channel)
{
  struct epoll_event event;
  ::memset(&event, 0, sizeof(event));
  event.events = channel->events();
  event.data.ptr = channel;
  if (::epoll_ctl(epollfd_, op, channel->fd(), &event) < 0) {
    LOG_ERROR << "epoll_ctl error: "
              << ::strerror(errno) << " for " << channel;
  }
}

Poller *Poller::newEpollPoller(EventLoop *loop)
{
  return new EpollPoller(loop);
}

} // namespace net

} // namespace ycnt
