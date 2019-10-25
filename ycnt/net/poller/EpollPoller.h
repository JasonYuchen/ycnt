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
  static constexpr int kInitEventsSize = 16;
  using ChannelVec = std::vector<Channel *>;

  explicit EpollPoller(EventLoop *loop);
  ~EpollPoller() final;
  base::Timestamp poll(int timeoutMs, ChannelVec &activeChannels) override;
  void updateChannel(Channel *channel) override;
  void removeChannel(Channel *channel) override;
  bool hasChannel(Channel *channel) const override;
 private:
  DISALLOW_COPY_AND_ASSIGN(EpollPoller);
  using ChannelMap = std::map<int, Channel *>;
  using EventVec = std::vector<struct epoll_event>;
  void updateInEpoll(int op, Channel *channel);
  EventLoop *loop_;
  int epollfd_;
  EventVec events_;
  ChannelMap channels_;
};

} // namespace net

} // namespace ycnt

#endif //YCNT_YCNT_NET_POLLER_EPOLLPOLLER_H_
