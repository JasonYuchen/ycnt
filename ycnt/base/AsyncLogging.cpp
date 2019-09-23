//
// Created by jason on 2019/9/19.
//

#include <assert.h>

#include <ycnt/base/AsyncLogging.h>
#include <ycnt/base/Timestamp.h>
#include <ycnt/base/File.h>

using namespace std;

namespace ycnt
{

namespace base
{

class AsyncLogging::LogBufferPool {
 public:
  explicit LogBufferPool(size_t initSize = 4, size_t maxSize = 16)
    : maxPoolSize_(maxSize)
  {
    for (int i = 0; i < initSize; ++i) {
      emptyBuffers_.push(std::make_unique<Buffer>());
    }
  }
  ~LogBufferPool() = default;
  BufferPtr pop()
  {
    auto buf = emptyBuffers_.try_pop();
    if (buf) {
      return buf;
    } else {
      return make_unique<Buffer>();
    }
  }
  void push(BufferPtr buf)
  {
    if (emptyBuffers_.size() < 16) {
      emptyBuffers_.push(std::move(buf));
    }
  }
 private:
  DISALLOW_COPY_MOVE_AND_ASSIGN(LogBufferPool);
  BlockingQueue<BufferPtr> emptyBuffers_;
  size_t maxPoolSize_;
};

AsyncLogging::AsyncLogging(
  const std::string &basename,
  size_t bucketSize,
  size_t rollSize,
  int flushInterval)
  : basename_(basename),
    rollSize_(rollSize),
    flushInterval_(flushInterval),
    running_(false),
    thread_(
      [this]
      { this->threadFunc(); }, "AsyncLogging"),
    latch_(1),
    mutex_(),
    cv_(),
    fixedBuffers_(bucketSize),
    frontBuffers_(),
    bufferPool_(new LogBufferPool())
{
  for (auto &node : fixedBuffers_) {
    node.buffer_.reset(new Buffer());
  }
}

AsyncLogging::~AsyncLogging()
{
  if (running_) {
    stop();
  }
  delete bufferPool_;
}

void AsyncLogging::append(const char *logline, int len)
{
  int index = currentThread::tid() % fixedBuffers_.size();
  auto &node = fixedBuffers_[index];
  lock_guard<mutex> nodeGuard(node.mutex_);
  if (node.buffer_->avail() > len) {
    node.buffer_->append(logline, len);
  } else {
    {
      // add full buffer to the frontBuffers
      lock_guard<mutex> frontGuard(mutex_);
      frontBuffers_.emplace_back(node.buffer_.release());
      cv_.notify_all();
    }
    // fetch an empty buffer from pool
    node.buffer_ = std::move(bufferPool_->pop());
    assert(node.buffer_);
    node.buffer_->append(logline, len);
  }
}

void AsyncLogging::threadFunc()
{
  assert(running_ == true);
  latch_.countDown();
  LogFile output(basename_, rollSize_);
  BufferPtrVec backBuffers_;
  backBuffers_.reserve(16);
  while (running_) {
    assert(backBuffers_.empty());
    {
      unique_lock<mutex> frontGuard(mutex_);
      if (frontBuffers_.empty()) {
        cv_.wait_for(frontGuard, std::chrono::seconds(flushInterval_));
      }
    }
    // lock order: node->front
    for (auto &node : fixedBuffers_) {
      lock_guard<mutex> nodeGuard(node.mutex_);
      if (node.buffer_->length() > 0) {
        lock_guard<mutex> frontGuard(mutex_);
        frontBuffers_.emplace_back(node.buffer_.release());
        node.buffer_ = std::move(bufferPool_->pop());
      }
    }
    {
      lock_guard<mutex> frontGuard(mutex_);
      backBuffers_.swap(frontBuffers_);
    }
    if (backBuffers_.size() > 25) {
      char buf[256];
      snprintf(
        buf, sizeof(buf),
        "Dropped log messages at %s, %zd larger buffers\n",
        Timestamp::now().toFormattedString().c_str(),
        backBuffers_.size() - 2);
      fputs(buf, stderr);
      output.append(buf, strlen(buf));
      backBuffers_.erase(backBuffers_.begin() + 2, backBuffers_.end());
    }
    for (const auto &buffer : backBuffers_) {
      output.append(buffer->data(), buffer->length());
      buffer->reset();
    }
    for (int i = 0; i < backBuffers_.size(); ++i) {
      bufferPool_->push(std::move(backBuffers_.back()));
      backBuffers_.pop_back();
    }
    backBuffers_.clear();
    output.flush();
  }
  output.flush();
}

} // namespace base

} // namespace ycnt
