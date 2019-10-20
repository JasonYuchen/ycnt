//
// Created by jason on 2019/9/29.
//

#ifndef YCNT_YCNT_BASE_THREADPOOL_H_
#define YCNT_YCNT_BASE_THREADPOOL_H_

#include <functional>
#include <future>
#include <memory>
#include <queue>

#include <ycnt/base/Thread.h>
#include <ycnt/base/Types.h>
#include <ycnt/base/ThreadSafeContainer.h>

namespace ycnt
{

namespace base
{

using Task = std::function<void()>;

class ThreadPool {
 public:
  // RAN = random, RR = round robin, LB0 = load balancing-0
  enum SchedulePolicy { RAN, RR, LB0 };
  static std::unique_ptr<ThreadPool> newThreadPool(
    size_t threadNum,
    const std::string &name = "ThreadPool",
    SchedulePolicy policy = RAN,
    const Task &threadInitCallback = []
    {},
    size_t maxQueueSize = 100000
  );

  ~ThreadPool();
  void start();
  void stop();
  const std::string &name() const;

  // do not throw exception in f which may cause std::abort
  // if workerId is specified, scheduling will be suppressed and the submitted
  // job will be executed in thread workers[workerId % workerSize]
  void run(Task f, int workerId = -1);

  // can throw exception which will be wrapped in the future
  //  do we need an interface with any return type?
  template<typename Func>
  std::future<typename std::result_of<Func()>::type> waitableRun(
    Func f,
    int workerId = -1)
  {
    using resultType = typename std::result_of<Func()>::type;
    auto
      task = std::make_shared<std::packaged_task<resultType()>>(std::move(f));
    std::future<resultType> result(task->get_future());
    if (workers_.empty()) {
      task->operator()();
    } else {
      run(
        [task = std::move(task)]
        { task->operator()(); },
        workerId);
    }
    return result;
  }

 private:
  ThreadPool(
    const std::string &name,
    SchedulePolicy policy,
    const Task &initCallback,
    size_t maxQueueSize);
  DISALLOW_COPY_AND_ASSIGN(ThreadPool);
  const std::string name_;
  const SchedulePolicy policy_;
  Task initCallback_; // should be reentrant
  class Worker;
  std::vector<std::unique_ptr<Worker>> workers_;
  std::atomic_bool running_;
};

} // namespace base

} // namespace ycnt


#endif //YCNT_YCNT_BASE_THREADPOOL_H_
