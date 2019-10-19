//
// Created by jason on 2019/9/27.
//

#include <sys/epoll.h>
#include <unistd.h>

#include <ycnt/net/poller/Poller.h>
#include <ycnt/net/poller/EpollPoller.h>
#include <ycnt/base/LogStream.h>
#include <ycnt/net/Channel.h>

namespace ycnt
{

namespace net
{

// TODO: EpollPoller

EpollPoller::EpollPoller(EventLoop *loop)
  : loop_(loop),
    epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
    events_(kInitEventsSize)
{
  if (epollfd_ < 0) {
    LOG_FATAL << "::epoll_create1 returned " << epollfd_;
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
  // TODO
}

void EpollPoller::updateChannel(Channel *channel)
{
  loop_->assertInLoopThread();
  Channel::Flag flag = channel->flag();
  // TODO
}

void EpollPoller::removeChannel(Channel *channel)
{
  loop_->assertInLoopThread();
  // TODO
}

bool EpollPoller::hasChannel(Channel *channel) const
{
  loop_->assertInLoopThread();
  return channels_.find(channel->fd()) != channels_.end();
}

Poller *Poller::newEpollPoller(EventLoop *loop)
{
  return new EpollPoller(loop);
}

} // namespace net

} // namespace ycnt
