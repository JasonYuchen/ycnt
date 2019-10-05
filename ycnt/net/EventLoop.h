//
// Created by jason on 2019/10/2.
//

#ifndef YCNT_YCNT_NET_EVENTLOOP_H_
#define YCNT_YCNT_NET_EVENTLOOP_H_

#include <atomic>
#include <functional>
#include <memory>

#include <ycnt/net/poller/Poller.h>
#include <ycnt/base/Types.h>

namespace ycnt
{

namespace net
{

class EventLoop {
 public:
  EventLoop();
  ~EventLoop();
  void loop();
  void assertInLoopThread();
 private:
  DISALLOW_COPY_AND_ASSIGN(EventLoop);
  const int threadId_;
  bool looping_;
  std::atomic_bool quit_;
  bool eventHandling_;
  bool functorHandling_;
  int64_t iteration_;
  std::unique_ptr<Poller> poller_;
  std::vector<std::function<void()>> functors_;
};

} // namespace net

} // namespace ycnt

#endif //YCNT_YCNT_NET_EVENTLOOP_H_
