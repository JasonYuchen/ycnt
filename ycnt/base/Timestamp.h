//
// Created by jason on 2019/9/23.
//

#ifndef YCNT_YCNT_BASE_TIMESTAMP_H_
#define YCNT_YCNT_BASE_TIMESTAMP_H_

#include <stdint.h>
#include <algorithm>
#include <string>

namespace ycnt
{

namespace base
{

class Timestamp {
 public:
  Timestamp() : microSecondsSinceEpoch_(0)
  {}
  explicit Timestamp(int64_t us) : microSecondsSinceEpoch_(us)
  {}
  void swap(Timestamp &rhs)
  {
    std::swap(microSecondsSinceEpoch_, rhs.microSecondsSinceEpoch_);
  }
  std::string toString() const;
  std::string toFormattedString(bool showMicroseconds = true) const;
  bool valid() const
  { return microSecondsSinceEpoch_ > 0; }

  int64_t microSecondsSinceEpoch() const
  { return microSecondsSinceEpoch_; }
  time_t secondsSinceEpoch() const
  {
    return static_cast<time_t>(microSecondsSinceEpoch_
      / kMicroSecondsPerSecond);
  }

  static Timestamp now();
  static Timestamp invalid()
  {
    return {};
  }
  static Timestamp fromUnixTime(time_t t)
  {
    return fromUnixTime(t, 0);
  }
  static Timestamp fromUnixTime(time_t t, int microseconds)
  {
    return Timestamp(
      static_cast<int64_t>(t) * kMicroSecondsPerSecond + microseconds);
  }
  static const int kMicroSecondsPerSecond = 1000 * 1000;

 private:
  int64_t microSecondsSinceEpoch_;
};

inline bool operator<(Timestamp lhs, Timestamp rhs)
{
  return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}

inline bool operator==(Timestamp lhs, Timestamp rhs)
{
  return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}

} // namespace base

} // namespace ycnt

#endif //YCNT_YCNT_BASE_TIMESTAMP_H_
