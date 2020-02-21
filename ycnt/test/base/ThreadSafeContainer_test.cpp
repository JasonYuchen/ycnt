//
// Created by jason on 2019/9/21.
//

#include <iostream>
#include <gtest/gtest.h>
#include <vector>
#include <optional>

#include <ycnt/base/ThreadSafeContainer.h>
#include <ycnt/base/Thread.h>
#include <ycnt/base/Timestamp.h>

using namespace std;
using namespace ycnt::base;

TEST(ThreadSafeQueue, BlockingQueue)
{
  vector<int> a{1, 2, 3};
  std::for_each(
    a.begin(), a.end(), [](int &ch)
    { ch = -ch; });
  cout << "ok";
}

