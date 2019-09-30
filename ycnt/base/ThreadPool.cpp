//
// Created by jason on 2019/9/29.
//

#include <exception>

#include <ycnt/base/ThreadPool.h>
#include <ycnt/base/Utils.h>

using namespace std;

namespace ycnt
{

namespace base
{

std::unique_ptr<ThreadPool> ThreadPool::newThreadPool(
  size_t threadNum,
  const std::string &name,
  const ycnt::base::ThreadPool::Task &threadInitCallback,
  size_t maxQueueSize)
{
  std::unique_ptr<ThreadPool> pool(new ThreadPool(name, maxQueueSize));
  pool->threadInitCallback_ = threadInitCallback;
  pool->threads_.reserve(threadNum);
  for (int i = 0; i < threadNum; ++i) {
    char id[32];
    convert(id, i + 1);
    pool->threads_.emplace_back(
      make_unique<Thread>(
        [&pool]()
        { pool->runInThread(); },
        pool->name_ + id));
  }
  return pool;
}

ThreadPool::ThreadPool(const std::string &name, size_t maxQueueSize)
  : name_(name),
    queue_(maxQueueSize),
    running_(false)
{
}

ThreadPool::~ThreadPool()
{
  if (running_) {
    stop();
  }
}

void ThreadPool::start()
{
  running_ = true;
  for (auto &thread : threads_) {
    thread->start();
  }
  if (threads_.empty()) {
    threadInitCallback_();
  }
}

void ThreadPool::stop()
{
  running_ = false;
  queue_.push(
    []
    {});
  for (auto &thread : threads_) {
    thread->join();
  }
}

const std::string &ThreadPool::name() const
{
  return name_;
}

size_t ThreadPool::queueSize() const
{
  return queue_.size();
}

void ThreadPool::run(Task task)
{
  if (threads_.empty()) {
    task();
  } else {
    queue_.push(std::move(task));
  }
}

void ThreadPool::runInThread()
{
  try {
    threadInitCallback_();
    while (running_) {
      Task task(queue_.pop());
      if (task) {
        task();
      }
    }
  } catch (const exception &ex) {
    fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
    fprintf(stderr, "reason: %s\n", ex.what());
    abort();
  } catch (...) {
    fprintf(
      stderr,
      "unknown exception caught in ThreadPool %s\n",
      name_.c_str());
    throw;
  }
}

} // namespace base

} // namespace ycnt
