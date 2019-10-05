//
// Created by jason on 2019/9/27.
//

#ifndef YCNT_YCNT_NET_POLLER_EPOLLPOLLER_H_
#define YCNT_YCNT_NET_POLLER_EPOLLPOLLER_H_

#include <vector>
#include <map>

#include <ycnt/net/poller/Poller.h>
#include <ycnt/base/Types.h>
#include <ycnt/base/Timestamp.h>

namespace ycnt
{

namespace net
{

class EventLoop;
class Channel;

class EpollPoller : public Poller {
 public:
  using ChannelVec = std::vector<Channel *>;

  EpollPoller(EventLoop *loop);
  ~EpollPoller();
  base::Timestamp poll(int timeoutMs, ChannelVec &activeChannels) override;
  void updateChannel(Channel *channel) override;
  void removeChannel(Channel *channel) override;
  bool hasChannel(Channel *channel) const override;
 private:
  DISALLOW_COPY_AND_ASSIGN(EpollPoller);
  using ChannelMap = std::map<int, Channel *>;
  ChannelMap channels_;
  EventLoop *loop_;
};

} // namespace net

} // namespace ycnt

#endif //YCNT_YCNT_NET_POLLER_EPOLLPOLLER_H_
