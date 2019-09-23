//
// Created by jason on 2019/9/19.
//

#ifndef YCNT_YCNT_BASE_COUNTDOWNLATCH_H_
#define YCNT_YCNT_BASE_COUNTDOWNLATCH_H_

#include <mutex>
#include <condition_variable>
#include <ycnt/base/Types.h>

namespace ycnt
{

namespace base
{

class CountDownLatch {
 public:
  explicit CountDownLatch(int count = 1) : mutex_(), cv_(), count_(count)
  {}

  void wait()
  {
    std::unique_lock<std::mutex> guard(mutex_);
    while (count_ > 0) {
      cv_.wait(guard);
    }
  }

  void countDown()
  {
    std::lock_guard<std::mutex> guard(mutex_);
    --count_;
    if (count_ == 0) {
      cv_.notify_all();
    }
  }

  int getCount() const
  {
    std::lock_guard<std::mutex> guard(mutex_);
    return count_;
  }
 private:
  DISALLOW_COPY_AND_ASSIGN(CountDownLatch);
  mutable std::mutex mutex_;
  std::condition_variable cv_;
  int count_;
};

} // namespace base

} // namespace ycnt

#endif //YCNT_YCNT_BASE_COUNTDOWNLATCH_H_
