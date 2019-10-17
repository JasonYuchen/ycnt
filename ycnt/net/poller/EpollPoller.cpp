//
// Created by jason on 2019/9/27.
//

#include <ycnt/net/poller/Poller.h>
#include <ycnt/net/poller/EpollPoller.h>

namespace ycnt
{

namespace net
{

// TODO: EpollPoller

EpollPoller::EpollPoller(ycnt::net::EventLoop *loop)
  : loop_(loop)
{

}

EpollPoller::~EpollPoller()
{

}

base::Timestamp EpollPoller::poll(
  int timeoutMs,
  ycnt::net::ChannelVec &activeChannels)
{

}

void EpollPoller::updateChannel(ycnt::net::Channel *channel)
{

}

void EpollPoller::removeChannel(ycnt::net::Channel *channel)
{

}

bool EpollPoller::hasChannel(ycnt::net::Channel *channel) const
{

}

Poller *Poller::newEpollPoller(ycnt::net::EventLoop *loop)
{
  return new EpollPoller(loop);
}

} // namespace net

} // namespace ycnt
