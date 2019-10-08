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
  // RAN = random, RR = round robin, LB = load balancing
  enum SchedulePolicy { RAN, RR, LB };
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
  void run(Task f);

  //  do we need an interface with any return type?
  template<typename Func>
  std::future<typename std::result_of<Func()>::type> waitableRun(Func f)
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
        { task->operator()(); });
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
