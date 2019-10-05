//
// Created by jason on 2019/9/27.
//

#include <ycnt/net/poller/Poller.h>
#include <ycnt/net/poller/EpollPoller.h>

namespace ycnt
{

namespace net
{

Poller *Poller::newEpollPoller(ycnt::net::EventLoop *loop)
{
  return new EpollPoller(loop);
}

} // namespace net

} // namespace ycnt
