//
// Created by jason on 2019/9/24.
//

#include <gtest/gtest.h>
#include <iostream>

#include <ycnt/base/AsyncLogging.h>
#include <ycnt/base/LogStream.h>
#include <ycnt/base/Utils.h>

using namespace std;

using namespace ycnt;
using namespace ycnt::base;

TEST(LogStream, LogStreamBasic)
{
  Logger::init("testLogger");
  Thread thread1(
    []
    {
      LOG_TRACE << "trace" << 1 << 'c' << 2.5;
      LOG_DEBUG << "DEBUG" << 5.6 << 's' << 0;
      LOG_ERROR << "error" << "brig";
      LOG_INFO << "INFO";
      LOG_WARN << "warning" << std::string("string--------------------");
    });
  Thread thread2(
    []
    {
      LOG_TRACE << "trace" << 1 << 'c' << 2.5;
      LOG_DEBUG << "DEBUG" << 5.6 << 's' << 0;
      LOG_ERROR << "error" << "brig";
      LOG_INFO << "INFO";
      LOG_WARN << "warning" << std::string("string--------------------");
    });
  Thread thread3(
    []
    {
      LOG_TRACE << "trace" << 1 << 'c' << 2.5;
      LOG_DEBUG << "DEBUG" << 5.6 << 's' << 0;
      LOG_ERROR << "error" << "brig";
      LOG_INFO << "INFO";
      LOG_WARN << "warning" << std::string("string--------------------");
    });
  thread1.start();
  thread2.start();
  thread3.start();
  LOG_TRACE << "trace" << 1 << 'c' << 2.5;
  LOG_DEBUG << "DEBUG" << 5.6 << 's' << 0;
  LOG_ERROR << "error" << "brig";
  LOG_INFO << "INFO";
  LOG_WARN << "warning" << std::string("string--------------------");
  thread1.join();
  thread2.join();
  thread3.join();
  sleep(3);
}

TEST(LogStream, BenchFromMuduo)
{
  Logger::init("testLogger", 1, 500 * 1000 * 1000);
  int cnt = 0;
  const int kBatch = 1000;
  string empty = " ";
  for (int t = 0; t < 10; ++t) {
    Timestamp start = Timestamp::now();
    for (int i = 0; i < kBatch; ++i) {
      LOG_INFO << "Hello 0123456789" << " abcdefghijklmnopqrstuvwxyz "
               << empty
               << cnt;
      ++cnt;
    }
    Timestamp end = Timestamp::now();
    printf("%f\n", timeDifference(end, start) * 1000000 / kBatch);
    struct timespec ts = {0, 500 * 1000 * 1000};
    nanosleep(&ts, NULL);
  }
  sleep(3);
}