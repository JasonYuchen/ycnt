//
// Created by jason on 2019/9/21.
//

#ifndef YCNT_YCNT_BASE_BUFFER_H_
#define YCNT_YCNT_BASE_BUFFER_H_

#include <string.h>
#include <stddef.h>
#include <algorithm>

#include <ycnt/base/Types.h>

namespace ycnt
{

namespace base
{

constexpr int kSmallBufferSize = 4096;
constexpr int kLargeBufferSize = 4096 * 1024;

template<int SIZE>
class FixedBuffer {
 public:
  FixedBuffer() : cur_(data_), end_(data_ + SIZE)
  {
    setCookie(cookieStart);
  }
  ~FixedBuffer()
  {
    setCookie(cookieEnd);
  }

  size_t append(const char *buf, size_t len)
  {
    size_t actualLen = std::min(avail(), len);
    ::memcpy(cur_, buf, actualLen);
    cur_ += actualLen;
  }

  const char *data() const
  {
    return data_;
  }

  char *current()
  {
    return cur_;
  }

  void add(size_t len)
  {
    cur_ += len;
  }

  void reset()
  {
    cur_ = data_;
  }

  void bzero()
  {
    ::memset(data_, 0, SIZE);
  }

  size_t length() const
  {
    return cur_ - data_;
  }

  size_t avail() const
  {
    return end_ - cur_;
  }

  void setCookie(void (*cookie)())
  {
    cookie_ = cookie;
  }
 private:
  DISALLOW_COPY_MOVE_AND_ASSIGN(FixedBuffer);
  static void cookieStart()
  {}
  static void cookieEnd()
  {}
  void (*cookie_)();
  char data_[SIZE];
  char *cur_;
  const char *const end_;
};

} // namespace base

} // namespace ycnt

#endif //YCNT_YCNT_BASE_BUFFER_H_
