//
// Created by jason on 2019/9/18.
//

#ifndef YCNT_YCNT_BASE_TYPES_H_
#define YCNT_YCNT_BASE_TYPES_H_

#define CACHE_LINE 64

#define LIKELY(x) (__builtin_expect((x), 1))
#define UNLIKELY(x) (__builtin_expect((x), 0))

#define DEFAULT_COPY_AND_ASSIGN(TypeName)           \
TypeName(const TypeName&) = default;                \
TypeName& operator=(const TypeName&) = default      \

#define DISALLOW_COPY_AND_ASSIGN(TypeName)          \
TypeName(const TypeName&) = delete;                 \
TypeName& operator=(const TypeName&) = delete

#define DISALLOW_COPY_MOVE_AND_ASSIGN(TypeName)     \
TypeName(const TypeName&) = delete;                 \
TypeName& operator=(const TypeName&) = delete;      \
TypeName(TypeName&&) = delete;                      \
TypeName& operator=(const TypeName&&) = delete

class StringArg {
 public:
  template<int N>
  StringArg(const char (&str)[N]) : ptr_(str), len_(N - 1)
  {}
  StringArg(const char *str, size_t len) : ptr_(str), len_(len)
  {}
  StringArg(const StringArg &) = default;
  StringArg &operator=(const StringArg &) = default;

  const char *data() const
  {
    return ptr_;
  }

  size_t size() const
  {
    return len_;
  }
 private:
  const char *ptr_;
  size_t len_;
};

inline StringArg operator ""_arg(const char *str, size_t len)
{
  return StringArg(str, len);
}

#endif //YCNT_YCNT_BASE_TYPES_H_
