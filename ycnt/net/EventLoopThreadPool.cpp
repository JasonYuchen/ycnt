//
// Created by jason on 2019/10/29.
//


#include <assert.h>
#include <ycnt/net/EventLoopThreadPool.h>
#include <ycnt/net/EventLoop.h>

using namespace std;
using namespace ycnt::base;

namespace ycnt
{

namespace net
{

EventLoopThreadPool::EventLoopThreadPool(
  ycnt::net::EventLoop *baseLoop,
  size_t numThreads,
  string_view name)
  : baseLoop_(baseLoop),
    numThreads_(numThreads),
    name_(name),
    started_(false)
{
}

EventLoopThreadPool::~EventLoopThreadPool()
{
}

void EventLoopThreadPool::start(const ThreadInitCallback &cb)
{
  assert(!started_);
  baseLoop_->assertInLoopThread();
  started_ = true;
  for (int i = 0; i < numThreads_; ++i) {
    EventLoop *loop;
    Thread *thread = new Thread(
      [callback_ = std::move(cb), loop_ = &loop]()
      {
        EventLoop loop;
        if (callback_) {
          callback_(&loop);
        }

        {

        }

      },
      name_ + to_string(i)
    );
  }
}

} // namespace net

} // namespace ycnt
