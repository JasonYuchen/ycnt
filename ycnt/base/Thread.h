//
// Created by jason on 2019/9/19.
//

#ifndef YCNT_YCNT_BASE_THREAD_H_
#define YCNT_YCNT_BASE_THREAD_H_

#include <atomic>
#include <functional>
#include <string>
#include <pthread.h>

#include <ycnt/base/Types.h>
#include <ycnt/base/CountDownLatch.h>

namespace ycnt
{

namespace currentThread
{

extern __thread int t_cachedTid;
extern __thread char t_tidString[32];
extern __thread int t_tidStringLength;
extern __thread const char *t_threadName;

void cacheTid();

inline int tid()
{
  if (UNLIKELY(t_cachedTid == 0)) {
    cacheTid();
  }
  return t_cachedTid;
}

inline const char *tidString()
{
  return t_tidString;
}

inline int tidStringLength()
{
  return t_tidStringLength;
}

inline const char *name()
{
  return t_threadName;
}

bool isMainThread();

} // namespace currentThread

namespace base
{

class Thread {
 public:
  using ThreadFunc = std::function<void()>;
  explicit Thread(ThreadFunc, const std::string &name = "unknown");
  ~Thread();
  void start();
  int join();
  bool started() const;
  pid_t tid() const;
  const std::string &name() const;
  static int numCreated();
 private:
  DISALLOW_COPY_AND_ASSIGN(Thread);
  bool started_;
  bool joined_;
  pthread_t pthreadId_;
  pid_t tid_;
  ThreadFunc func_;
  std::string name_;
  CountDownLatch latch_;
  static std::atomic_int numCreated_;
};

} // namespace base

} // namespace ycnt

#endif //YCNT_YCNT_BASE_THREAD_H_
