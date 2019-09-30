//
// Created by jason on 2019/9/29.
//

#ifndef YCNT_YCNT_BASE_THREADPOOL_H_
#define YCNT_YCNT_BASE_THREADPOOL_H_

#include <functional>

#include <ycnt/base/Thread.h>
#include <ycnt/base/Types.h>
#include <ycnt/base/ThreadSafeContainer.h>

namespace ycnt
{

namespace base
{

class ThreadPool {
 public:
  using Task = std::function<void()>;
  static std::unique_ptr<ThreadPool> newThreadPool(
    size_t threadNum,
    const std::string &name = "ThreadPool",
    const Task &threadInitCallback = []
    {},
    size_t maxQueueSize = 100000
  );

  ~ThreadPool();
  void start();
  void stop();
  const std::string &name() const;
  size_t queueSize() const;
  void run(Task f);

//  TODO: do we need an interface with any return type?
//  template<typename Func>
//  std::future<typename std::result_of<Func()>::type> run(Func f)
//  {
//    using resultType = typename std::result_of<Func()>::type;
//    std::packaged_task<resultType()> task(std::move(f));
//    std::future<resultType> result(task.get_future());
//    queue_.push(std::move(task));
//    return result;
//  }

 private:
  ThreadPool(const std::string &name, size_t maxQueueSize);
  DISALLOW_COPY_AND_ASSIGN(ThreadPool);
  void runInThread();
  const std::string name_;
  Task threadInitCallback_; // should be reentrant
  std::vector<std::unique_ptr<Thread>> threads_;
  BoundedBlockingQueue<Task> queue_;
  std::atomic_bool running_;
};

} // namespace base

} // namespace ycnt


#endif //YCNT_YCNT_BASE_THREADPOOL_H_
