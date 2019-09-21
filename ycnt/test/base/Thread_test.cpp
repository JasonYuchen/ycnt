//
// Created by jason on 2019/9/19.
//

#include <iostream>
#include <gtest/gtest.h>

#include <ycnt/base/Thread.h>

using namespace ycnt;

TEST(Thread, StartFunction)
{
  std::atomic_int value(0);
  base::CountDownLatch latch;
  ASSERT_EQ(latch.getCount(), 1);
  base::Thread thread1(
    [&]
    {
      value.fetch_add(1);
      latch.countDown();
    }, "thread1");
  ASSERT_EQ(base::Thread::numCreated(), 1);
  ASSERT_FALSE(thread1.started());
  base::Thread thread2(
    [&]
    {
      latch.wait();
      value.fetch_add(1);
    });
  ASSERT_EQ(value, 0);
  thread2.start();
  std::cout << thread2.name() << " started with tid = " << thread2.tid()
            << std::endl;
  ASSERT_TRUE(thread2.started());
  ASSERT_EQ(value, 0);
  thread1.start();
  std::cout << thread1.name() << " started with tid = " << thread1.tid()
            << std::endl;
  thread2.join();
  ASSERT_EQ(value, 2);
}