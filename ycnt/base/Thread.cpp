//
// Created by jason on 2019/9/19.
//

#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>
#include <exception>
#include <assert.h>

#include <ycnt/base/Thread.h>

using namespace std;

namespace ycnt
{

pid_t gettid()
{
  return static_cast<pid_t>(::syscall(SYS_gettid));
}

namespace currentThread
{

__thread int t_cachedTid = 0;
__thread char t_tidString[32];
__thread int t_tidStringLength = 6;
__thread const char *t_threadName = "unknown";

void cacheTid()
{
  if (t_cachedTid == 0) {
    t_cachedTid = gettid();
    t_tidStringLength =
      snprintf(t_tidString, sizeof(t_tidString), "%5d ", t_cachedTid);
  }
}

bool isMainThread()
{
  return tid() == ::getpid();
}

} // namespace currentThread

namespace base
{

struct ThreadData {
  Thread::ThreadFunc func_;
  string name_;
  pid_t *tid_;
  CountDownLatch *latch_;
  ThreadData(
    Thread::ThreadFunc &&func,
    const string &name,
    pid_t *tid,
    CountDownLatch *latch)
    : func_(std::move(func)),
      name_(name),
      tid_(tid),
      latch_(latch)
  {}

  // to capture exceptions
  void run()
  {
    *tid_ = currentThread::tid();
    tid_ = nullptr;
    latch_->countDown();
    latch_ = nullptr;
    currentThread::t_threadName = name_.c_str();
    try {
      func_();
      currentThread::t_threadName = "finished";
    } catch (const exception &ex) {
      currentThread::t_threadName = "crashed";
      fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
      fprintf(stderr, "reason: %s\n", ex.what());
      abort();
    } catch (...) {
      currentThread::t_threadName = "crashed";
      fprintf(stderr, "unknown exception caught in Thread %s\n", name_.c_str());
      throw;
    }
  }
};

void *startThreadFunc(void *arg)
{
  auto data = static_cast<ThreadData *>(arg);
  data->run();
  delete data;
  return nullptr;
}

Thread::Thread(ThreadFunc func, const string &name)
  : started_(false),
    joined_(false),
    pthreadId_(0),
    tid_(0),
    func_(std::move(func)),
    name_(),
    latch_(1)
{
  name_ = std::to_string(numCreated_.fetch_add(1)) + ":" + name;
}

Thread::~Thread()
{
  if (started_ && !joined_) {
    pthread_detach(pthreadId_);
  }
}

void Thread::start()
{
  assert(!started_);
  started_ = true;
  auto data = new ThreadData(std::move(func_), name_, &tid_, &latch_);
  if (pthread_create(&pthreadId_, nullptr, &startThreadFunc, data)) {
    started_ = false;
    delete data;
    // LOG_FATAL << "Failed to start Thread " << name_ << " in pthread_create";
  } else {
    // wait for starting thread execution
    latch_.wait();
    assert(tid_ > 0);
  }
}

int Thread::join()
{
  assert(started_);
  assert(!joined_);
  joined_ = true;
  return pthread_join(pthreadId_, nullptr);
}

bool Thread::started() const
{
  return started_;
}

pid_t Thread::tid() const
{
  return tid_;
}

const string &Thread::name() const
{
  return name_;
}

int Thread::numCreated()
{
  return numCreated_;
}

std::atomic_int Thread::numCreated_(0);

} // namespace base

} // namespace ycnt