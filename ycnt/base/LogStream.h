//
// Created by jason on 2019/9/23.
//

#ifndef YCNT_YCNT_BASE_LOGSTREAM_H_
#define YCNT_YCNT_BASE_LOGSTREAM_H_

#include <string>

#include <ycnt/base/Types.h>
#include <ycnt/base/Buffer.h>
#include <ycnt/base/Timestamp.h>
#include <ycnt/base/AsyncLogging.h>

namespace ycnt
{

namespace base
{

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

class LogStream {
 public:
  using Buffer = FixedBuffer<kSmallBufferSize>;
  LogStream() = default;
  LogStream &operator<<(bool v);
  LogStream &operator<<(char v);
  LogStream &operator<<(unsigned char v);
  LogStream &operator<<(short v);
  LogStream &operator<<(unsigned short v);
  LogStream &operator<<(int v);
  LogStream &operator<<(unsigned int v);
  LogStream &operator<<(long v);
  LogStream &operator<<(unsigned long v);
  LogStream &operator<<(long long v);
  LogStream &operator<<(unsigned long long v);
  LogStream &operator<<(float v);
  LogStream &operator<<(double v);
  LogStream &operator<<(const char *v);
  LogStream &operator<<(const unsigned char *v);
  LogStream &operator<<(const std::string &v);
  LogStream &operator<<(const StringArg &v);
  LogStream &operator<<(const Buffer &v);

  void append(const char *data, size_t len)
  {
    buffer_.append(data, len);
  }

  const Buffer &buffer() const
  {
    return buffer_;
  }

  void resetBuffer()
  {
    buffer_.reset();
  }
 private:
  DISALLOW_COPY_AND_ASSIGN(LogStream);
  template<typename T>
  void formatNumeric(T);
  Buffer buffer_;
  static const int kMaxNumericSize = 32;
};

class Logger {
 public:
  enum LogLevel {
    TRACE, DEBUG, INFO, WARN, ERROR, FATAL, NUM_LOG_LEVELS,
  };
  Logger(StringArg file, int line, LogLevel level);
  ~Logger();

  LogStream &stream();
  // not thread safe, should be called in the beginning of the program
  static bool init(
    const std::string &basename,
    size_t bucketSize = 4,
    size_t rollSize = 512 * 1024 * 1024,
    int flushInterval = 3);
 private:
  static AsyncLogging *asyncOutput_;

  StringArg getBasename(StringArg filename);
  void formatTime();
  StringArg basename_;
  Timestamp time_;
  LogLevel level_;
  int line_;
  LogStream stream_;
};

} // namespace base

#define LOG_TRACE ycnt::base::Logger(__FILE__, __LINE__, ycnt::base::Logger::TRACE).stream()
#define LOG_DEBUG ycnt::base::Logger(__FILE__, __LINE__, ycnt::base::Logger::DEBUG).stream()
#define LOG_INFO ycnt::base::Logger(__FILE__, __LINE__, ycnt::base::Logger::INFO).stream()
#define LOG_WARN ycnt::base::Logger(__FILE__, __LINE__, ycnt::base::Logger::WARN).stream()
#define LOG_ERROR ycnt::base::Logger(__FILE__, __LINE__, ycnt::base::Logger::ERROR).stream()
#define LOG_FATAL ycnt::base::Logger(__FILE__, __LINE__, ycnt::base::Logger::FATAL).stream()

} // namespace ycnt


#endif //YCNT_YCNT_BASE_LOGSTREAM_H_
