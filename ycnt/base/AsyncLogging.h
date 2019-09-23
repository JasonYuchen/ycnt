//
// Created by jason on 2019/9/19.
//

#ifndef YCNT_YCNT_BASE_ASYNCLOGGING_H_
#define YCNT_YCNT_BASE_ASYNCLOGGING_H_

#include <vector>
#include <mutex>
#include <memory>
#include <string>
#include <atomic>
#include <thread>

#include <ycnt/base/Types.h>
#include <ycnt/base/CountDownLatch.h>
#include <ycnt/base/ThreadSafeContainer.h>
#include <ycnt/base/Thread.h>
#include <ycnt/base/Buffer.h>

namespace ycnt
{

namespace base
{

class AsyncLogging {
 public:
  AsyncLogging(
    const std::string &basename,
    size_t bucketSize,
    size_t rollSize,
    int flushInterval);
  ~AsyncLogging()
  {
    if (running_) {
      stop();
    }
  }

  void append(const char *logline, int len);
  void start()
  {
    running_ = true;
    thread_.start();
    latch_.wait();
  }
  void stop()
  {
    running_ = false;
    cv_.notify_all();
    thread_.join();
  }
 private:
  DISALLOW_COPY_AND_ASSIGN(AsyncLogging);
  using Buffer = FixedBuffer<kLargeBufferSize>;
  using BufferPtr = std::unique_ptr<Buffer>;
  using BufferPtrVec = std::vector<BufferPtr>;

  struct FixedBufferNode {
    std::mutex mutex_; // __attribute__ ((aligned(CACHE_LINE)));
    BufferPtr buffer_; // __attribute__ ((aligned(CACHE_LINE)));
  };

  void threadFunc();
  const std::string basename_;
  const size_t rollSize_;
  const int flushInterval_;
  std::atomic<bool> running_;
  Thread thread_;
  CountDownLatch latch_;
  std::mutex mutex_;
  std::condition_variable cv_;
  std::vector<FixedBufferNode> fixedBuffers_;
  BufferPtrVec frontBuffers_; // guarded by mutex_
  class LogBufferPool;
  std::unique_ptr<LogBufferPool> bufferPool_; // pimpl
};

} // namespace base

} // namespace ycnt

#endif //YCNT_YCNT_BASE_ASYNCLOGGING_H_
