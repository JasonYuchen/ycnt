//
// Created by jason on 2019/10/29.
//

#ifndef YCNT_YCNT_NET_EVENTLOOPTHREADPOOL_H_
#define YCNT_YCNT_NET_EVENTLOOPTHREADPOOL_H_

#include <ycnt/base/Types.h>
#include <ycnt/base/Thread.h>

namespace ycnt
{

namespace net
{

class EventLoop;
class EventLoopThread {
 public:
  using ThreadInitCallback=std::function<void(EventLoop *)>;
  EventLoopThread(
    const ThreadInitCallback &cb = ThreadInitCallback(),
    const std::string &name = std::string());
  ~EventLoopThread();
  EventLoop *startLoop();
 private:
  DISALLOW_COPY_AND_ASSIGN(EventLoopThread);
  void threadFunc();

  EventLoop *loop_;
  base::Thread thread_;
};

class EventLoopThreadPool {
 public:
  using ThreadInitCallback=std::function<void(EventLoop *)>;
  EventLoopThreadPool(
    EventLoop *baseLoop,
    size_t numThreads,
    std::string_view name);
  ~EventLoopThreadPool();
  void start(const ThreadInitCallback &cb = ThreadInitCallback());
  EventLoop *getNextLoop();
  EventLoop *getLoop(size_t hash);
  bool started() const;
  const std::string &name() const;
 private:
  DISALLOW_COPY_AND_ASSIGN(EventLoopThreadPool);
  EventLoop *baseLoop_;
  std::string name_;
  bool started_;
  int numThreads_;
  std::vector<std::unique_ptr<base::Thread>> threads_;
  std::vector<EventLoop *> loops_;
};

} // namespace net

} // namespace ycnt

#endif //YCNT_YCNT_NET_EVENTLOOPTHREADPOOL_H_
