//
// Created by jason on 2019/9/23.
//

#include <gtest/gtest.h>
#include <iostream>

#include <ycnt/base/AsyncLogging.h>

using namespace std;
using namespace ycnt::base;

TEST(AsyncLogging, BasicWithTwoThreads)
{
  auto
    logger = make_unique<AsyncLogging>("testLogger", 4, 500 * 1000 * 1000, 3);
  logger->start();

  Thread thread1(
    [&logger]
    {
      string data("test data from thread 1\n");
      for (int i = 0; i < 100; ++i) {
        logger->append(data.c_str(), data.length());
      }
    }, "frontEnd1");
  Thread thread2(
    [&logger]
    {
      string data("test data from thread 2\n");
      for (int i = 0; i < 100; ++i) {
        logger->append(data.c_str(), data.length());
      }
    }, "frontEnd2");
  thread1.start();
  thread2.start();
  thread1.join();
  thread2.join();
  ::sleep(3);
  logger->stop();
  logger.reset();
}