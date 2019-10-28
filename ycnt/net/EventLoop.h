//
// Created by jason on 2019/10/2.
//

#ifndef YCNT_YCNT_NET_EVENTLOOP_H_
#define YCNT_YCNT_NET_EVENTLOOP_H_

#include <atomic>
#include <functional>
#include <memory>
#include <any>
#include <mutex>

#include <ycnt/net/poller/Poller.h>
#include <ycnt/base/Types.h>

namespace ycnt
{

namespace net
{

class EventLoop {
 public:
  using Functor = std::function<void()>;
  EventLoop();
  ~EventLoop();
  void loop();
  void quit();
  int64_t iteration() const;
  void setContext(const std::any &context);
  const std::any &getContext() const;
  std::any &getContext();
  void runInLoop(Functor cb);
  void queueInLoop(Functor cb);
  size_t queueSize() const;

  void wakeup();
  void updateChannel(Channel *channel);
  void removeChannel(Channel *channel);
  bool hasChannel(Channel *channel);
  void assertInLoopThread();
  bool isInLoopThread();
 private:
  DISALLOW_COPY_AND_ASSIGN(EventLoop);
  const int threadId_;
  bool looping_;
  std::atomic_bool quit_;
  bool eventHandling_;
  bool functorHandling_;
  int64_t iteration_;
  base::Timestamp pollReturnTime_;
  std::unique_ptr<Poller> poller_;
  int wakeupFd_;
  std::unique_ptr<Channel> wakeupChannel_;
  std::any context_;
  ChannelVec activeChannels_;
  Channel *currentActiveChannel_;
  mutable std::mutex mutex_;
  std::vector<std::function<void()>> functors_;
  size_t maxQueueSize_;
};

} // namespace net

} // namespace ycnt

#endif //YCNT_YCNT_NET_EVENTLOOP_H_
