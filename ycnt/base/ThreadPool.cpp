//
// Created by jason on 2019/9/29.
//

#include <exception>

#include <ycnt/base/ThreadPool.h>
#include <ycnt/base/Utils.h>
#include <ycnt/base/Timestamp.h>

using namespace std;

namespace ycnt
{

namespace base
{

constexpr double kAlpha = 0.9;

class ThreadPool::Worker {
 public:
  Worker(const std::string &name, const Task &initCallback, size_t maxQueueSize)
    : thread_(
    [this]
    { this->runInThread(); }, name),
      running_(false),
      tasks_(maxQueueSize),
      lastAvgTaskRuntime_(0),
      avgTaskRuntime_(0)
  {
    tasks_.push(initCallback);
  }

  ~Worker()
  {
    if (running_) {
      stop();
    }
  }

  void start()
  {
    running_ = true;
    thread_.start();
  }

  void stop()
  {
    running_ = false;
    tasks_.push(
      []
      {});
    thread_.join();
  }

  void submit(Task &&f)
  {
    tasks_.push(std::move(f));
  }

  int64_t workload()
  {
    return avgTaskRuntime_.load() * tasks_.size();
  }

  int64_t avgRuntime()
  {
    return avgTaskRuntime_.load();
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(Worker);
  void runInThread()
  {
    try {
      while (running_) {
        Task task(tasks_.pop());
        if (task) {
          int64_t start = Timestamp::now().microSecondsSinceEpoch() / 1000;
          task();
          int64_t end = Timestamp::now().microSecondsSinceEpoch() / 1000;
          lastAvgTaskRuntime_ = static_cast<int64_t>(
            (lastAvgTaskRuntime_ * alpha_ + end - start) / (1 + alpha_));
          avgTaskRuntime_.store(lastAvgTaskRuntime_);
        }
      }
    } catch (const exception &ex) {
      fprintf(
        stderr,
        "exception caught in ThreadPool Worker %s\n",
        thread_.name().c_str());
      fprintf(stderr, "reason: %s\n", ex.what());
      abort();
    } catch (...) {
      fprintf(
        stderr,
        "unknown exception caught in ThreadPool %s\n",
        thread_.name().c_str());
      throw;
    }
  }
  Thread thread_;
  std::atomic_bool running_;
  BoundedBlockingQueue<Task> tasks_;
  int64_t lastAvgTaskRuntime_; // ms
  std::atomic_int64_t avgTaskRuntime_; // ms
  const double alpha_ = kAlpha;
};

std::unique_ptr<ThreadPool> ThreadPool::newThreadPool(
  size_t threadNum,
  const std::string &name,
  SchedulePolicy policy,
  const Task &initCallback,
  size_t maxQueueSize)
{
  std::unique_ptr<ThreadPool>
    pool(new ThreadPool(name, policy, initCallback, maxQueueSize));
  pool->workers_.reserve(threadNum);
  for (size_t i = 0; i < threadNum; ++i) {
    char id[32];
    convert(id, i + 1);
    pool->workers_.emplace_back(
      make_unique<Worker>(
        name + ":" + std::to_string(i),
        initCallback,
        maxQueueSize));
  }
  return pool;
}

ThreadPool::ThreadPool(
  const std::string &name,
  SchedulePolicy policy,
  const Task &initCallback,
  size_t maxQueueSize)
  : name_(name),
    policy_(policy),
    initCallback_(initCallback),
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
  for (auto &worker : workers_) {
    worker->start();
  }
  if (workers_.empty()) {
    initCallback_();
  }
}

void ThreadPool::stop()
{
  running_ = false;
  for (auto &worker : workers_) {
    worker->stop();
  }
}

const std::string &ThreadPool::name() const
{
  return name_;
}

void ThreadPool::run(Task task, int workerId)
{
  if (workers_.empty()) {
    task();
  } else if (workerId >= 0) {
    workers_[workerId % workers_.size()]->submit(std::move(task));
  } else {
    // TODO: schedule, default=random
    static int idx = 0;
    switch (policy_) {
      case RAN: {
        idx = Timestamp::now().microSecondsSinceEpoch() % workers_.size();
        break;
      }
      case RR: {
        idx++;
        break;
      }
      case LB0: {
        int _idx = 0;
        int64_t minLoad = INT64_MAX;
        for (size_t i = 0; i < workers_.size(); ++i) {
          int64_t load = workers_[i]->workload();
          if (load < minLoad) {
            minLoad = load;
            _idx = i;
          }
        }
        idx = _idx;
        break;
      }
      default:idx = Timestamp::now().microSecondsSinceEpoch() % workers_.size();
    }
    workers_[idx]->submit(std::move(task));
  }
}

} // namespace base

} // namespace ycnt
