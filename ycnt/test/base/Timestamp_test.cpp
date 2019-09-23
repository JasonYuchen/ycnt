//
// Created by jason on 2019/9/23.
//

#include <iostream>
#include <gtest/gtest.h>

#include <ycnt/base/Timestamp.h>

using namespace std;
using namespace ycnt::base;

TEST(Timestamp, TimestampBase)
{
  Timestamp now(Timestamp::now());
  cout << now.toString() << "\n" << now.toFormattedString() << endl;
  ASSERT_TRUE(now.valid());
}