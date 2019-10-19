//
// Created by jason on 2019/10/6.
//

#ifndef YCNT_YCNT_NET_POLLER_POLLER_H_
#define YCNT_YCNT_NET_POLLER_POLLER_H_

#include <vector>

#include <ycnt/base/Timestamp.h>
#include <ycnt/net/EventLoop.h>

namespace ycnt
{

namespace net
{

class Channel;

using ChannelVec = std::vector<Channel *>;
class Poller {
 public:
  virtual ~Poller() = default;
  virtual base::Timestamp poll(int timeoutMs, ChannelVec &activeChannels) = 0;
  virtual void updateChannel(Channel *channel) = 0;
  virtual void removeChannel(Channel *channel) = 0;
  virtual bool hasChannel(Channel *channel) const = 0;
  static Poller *newEpollPoller(EventLoop *);
};

} // namespace net

} // namespace ycnt


#endif //YCNT_YCNT_NET_POLLER_POLLER_H_
