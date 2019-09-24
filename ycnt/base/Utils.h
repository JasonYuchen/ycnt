//
// Created by jason on 2019/9/22.
//

#ifndef YCNT_YCNT_BASE_UTILS_H_
#define YCNT_YCNT_BASE_UTILS_H_

#include <algorithm>

namespace ycnt
{

namespace base
{

// Efficient Integer to String Conversions, by Matthew Wilson.
// buf is assumed to have enough space
template<typename T>
inline size_t convert(char *buf, T value)
{
  static const char digits[] = "9876543210123456789";
  static const char * const zero = digits + 9;
  static_assert(sizeof(digits) == 20, "wrong digits");
  T i = value;
  char *p = buf;
  do {
    int lsd = static_cast<int>(i % 10);
    i /= 10;
    *p++ = zero[lsd];
  } while (i != 0);
  if (value < 0) {
    *p++ = '-';
  }
  *p = '\0';
  std::reverse(buf, p);
  return p - buf;
}

// Grisu2 by milo yip
size_t dtoa_milo(double value, char* buffer);

template<>
inline size_t convert(char *buf, double value)
{
  return dtoa_milo(value, buf);
}

} // namespace base

} // namespace ycnt



#endif //YCNT_YCNT_BASE_UTILS_H_
