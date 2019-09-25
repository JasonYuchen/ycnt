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