//
// Created by jason on 2019/9/18.
//

#ifndef YCNT_YCNT_BASE_SINGLETON_H_
#define YCNT_YCNT_BASE_SINGLETON_H_

#include <ycnt/base/Types.h>

namespace ycnt
{

namespace base
{

// C++11: the initialization of a static local variable is thread safe
template<typename T>
class Singleton {
 public:
  static T &instance()
  {
    static T value_;
    return value_;
  }
 private:
  DISALLOW_COPY_MOVE_AND_ASSIGN(Singleton);
};

} // namespace base

} // namespace ycnt

#endif //YCNT_YCNT_BASE_SINGLETON_H_
