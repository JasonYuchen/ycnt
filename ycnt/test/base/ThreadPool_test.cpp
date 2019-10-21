//
// Created by jason on 2019/10/7.
//

#include <iostream>
#include <memory>
#include <gtest/gtest.h>

#include <ycnt/base/ThreadPool.h>
#include <ycnt/base/Timestamp.h>

using namespace std;
using namespace ycnt;
using namespace ycnt::base;

TEST(ThreadPool, BasicRoutine)
{
  std::unique_ptr<ThreadPool> pool(
    ThreadPool::newThreadPool(
      4,
      "testThreadPool",
      ThreadPool::SchedulePolicy::RAN,
      []
      {
        cout << "worker thread " << currentThread::tid()
             << " is running..." << endl;
      }));
  pool->start();
  pool->run(
    []
    { cout << "normal job" << endl; });
  auto result = pool->waitableRun(
    []
    {
      cout << "future job" << endl;
      return 10;
    });
  int ret = result.get();
  auto ex = pool->waitableRun(
    []
    {
      cout << "exception job" << endl;
      throw std::runtime_error("?");
      return 10;
    });
  ASSERT_EQ(ret, 10);
  try {
    ex.get();
  } catch (std::runtime_error &ex) {
    ASSERT_STREQ(ex.what(), "?");
  }
  std::atomic_int num(0);
  for (int i = 0; i < 1000; ++i) {
    pool->run(
      [&num]
      {
        num.fetch_add(1);
      });
  }
  // wait for all task completion
  ::sleep(1);
  ASSERT_EQ(num, 1000);
  pool->stop();
}

TEST(ThreadPool, SpecifiyWorker)
{
  std::unique_ptr<ThreadPool> pool(
    ThreadPool::newThreadPool(
      4,
      "testThreadPool",
      ThreadPool::SchedulePolicy::RAN,
      []
      {
        cout << "worker thread " << currentThread::tid()
             << " is running..." << endl;
      }));
  pool->start();
  int a = 0;
  int b = 0;
  for (int i = 0; i < 100; ++i) {
    pool->run(
      [&a]
      { a++; }, 1);
  }
  for (int i = 0; i < 100; ++i) {
    pool->waitableRun(
      [&b]
      { b++; }, 1);
  }
  ::sleep(1);
  ASSERT_EQ(a, 100);
  ASSERT_EQ(b, 100);
}

TEST(ThreadPool, RoundRobin)
{
  std::unique_ptr<ThreadPool> pool(
    ThreadPool::newThreadPool(
      4,
      "testThreadPool",
      ThreadPool::SchedulePolicy::RR,
      []
      {
        cout << "worker thread " << currentThread::tid()
             << " is running..." << endl;
      }));
  pool->start();

  for (int i = 0; i < 10; ++i) {
    pool->run(
      []
      { cout << currentThread::tid() << endl; });
  }
}

TEST(ThreadPool, LoadBalance0)
{
  std::unique_ptr<ThreadPool> pool(
    ThreadPool::newThreadPool(
      4,
      "testThreadPool",
      ThreadPool::SchedulePolicy::LB0,
      []
      {
        cout << "worker thread " << currentThread::tid()
             << " is running..." << endl;
      }));
  pool->start();
  // TODO: use thread safe unordered map
  for (int i = 0; i < 10; ++i) {
    ::sleep(3);
    pool->run(
      []
      {
        ::sleep(currentThread::tid() % 5);
        cout << currentThread::tid() << endl;
      });
  }
}