//
// Created by jason on 2019/9/18.
//

#ifndef YCNT_YCNT_BASE_THREADSAFECONTAINER_H_
#define YCNT_YCNT_BASE_THREADSAFECONTAINER_H_

#include <atomic>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <memory>
#include <ycnt/base/Types.h>

namespace ycnt
{

namespace base
{

template<typename T>
class BlockingQueue {
 public:
  BlockingQueue() : mutex_(), cv_(), queue_()
  {}

  void push(T &&x)
  {
    std::lock_guard<std::mutex> guard(mutex_);
    queue_.push(std::move(x));
    cv_.notify_all();
  }

  void push(const T &x)
  {
    std::lock_guard<std::mutex> guard(mutex_);
    queue_.push(x);
    cv_.notify_all();
  }

  T pop()
  {
    std::unique_lock<std::mutex> guard(mutex_);
    while (queue_.empty()) {
      cv_.wait(guard);
    }
    T value = std::move(queue_.front());
    queue_.pop();
    return std::move(value);
  }

  // FIXME: T must have a well-defined default constructor
  T try_pop()
  {
    std::lock_guard<std::mutex> guard(mutex_);
    if (queue_.empty()) {
      return T();
    } else {
      T value = std::move(queue_.front());
      queue_.pop();
      return std::move(value);
    }
  }

  size_t size() const
  {
    std::lock_guard<std::mutex> guard(mutex_);
    return queue_.size();
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(BlockingQueue);
  mutable std::mutex mutex_;
  std::condition_variable cv_;
  std::queue<T> queue_;
};

} // namespace base

} // namespace ycnt

#endif //YCNT_YCNT_BASE_THREADSAFECONTAINER_H_
