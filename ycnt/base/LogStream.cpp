//
// Created by jason on 2019/9/23.
//

#include <string>
#include <assert.h>

#include <ycnt/base/LogStream.h>
#include <ycnt/base/Utils.h>
#include <ycnt/base/Thread.h>

using namespace std;

namespace ycnt
{

namespace base
{

template<typename T>
void LogStream::formatNumeric(T v)
{
  if (buffer_.avail() >= kMaxNumericSize) {
    size_t len = convert(buffer_.current(), v);
    buffer_.add(len);
  }
}

LogStream &LogStream::operator<<(bool v)
{
  buffer_.append(v ? "1" : "0", 1);
  return *this;
}

LogStream &LogStream::operator<<(char v)
{
  buffer_.append(&v, 1);
  return *this;
}

LogStream &LogStream::operator<<(unsigned char v)
{
  return operator<<(static_cast<unsigned int>(v));
}

LogStream &LogStream::operator<<(short v)
{
  return operator<<(static_cast<int>(v));
}

LogStream &LogStream::operator<<(unsigned short v)
{
  return operator<<(static_cast<unsigned int>(v));
}

LogStream &LogStream::operator<<(int v)
{
  formatNumeric(v);
  return *this;
}

LogStream &LogStream::operator<<(unsigned int v)
{
  formatNumeric(v);
  return *this;
}

LogStream &LogStream::operator<<(long v)
{
  formatNumeric(v);
  return *this;
}

LogStream &LogStream::operator<<(unsigned long v)
{
  formatNumeric(v);
  return *this;
}

LogStream &LogStream::operator<<(long long v)
{
  formatNumeric(v);
  return *this;
}

LogStream &LogStream::operator<<(unsigned long long v)
{
  formatNumeric(v);
  return *this;
}

LogStream &LogStream::operator<<(float v)
{
  return operator<<(static_cast<double>(v));
}

LogStream &LogStream::operator<<(double v)
{
  formatNumeric(v);
  return *this;
}

LogStream &LogStream::operator<<(const char *v)
{
  if (v) {
    buffer_.append(v, strlen(v));
  } else {
    buffer_.append("(null)", 6);
  }
  return *this;
}

LogStream &LogStream::operator<<(const unsigned char *v)
{
  return operator<<(reinterpret_cast<const char *>(v));
}

LogStream &LogStream::operator<<(const string &v)
{
  buffer_.append(v.data(), v.size());
  return *this;
}

LogStream &LogStream::operator<<(const StringArg &v)
{
  buffer_.append(v.data(), v.size());
  return *this;
}

LogStream &LogStream::operator<<(const Buffer &v)
{
  buffer_.append(v.data(), v.length());
  return *this;
}

const char *LogLevelName[Logger::NUM_LOG_LEVELS] =
  {
    "TRACE ",
    "DEBUG ",
    "INFO  ",
    "WARN  ",
    "ERROR ",
    "FATAL ",
  };

void defaultOutput(const char *str, size_t len)
{
  ::fwrite(str, 1, len, stdout);
}

void defaultFlush()
{
  ::fflush(stdout);
}

AsyncLogging *Logger::asyncOutput_ = nullptr;
Logger::LogLevel Logger::logLevel_ = INFO;

Logger::Logger(StringArg file, int line, LogLevel level)
  : basename_(getBasename(file)),
    time_(Timestamp::now()),
    level_(level),
    line_(line)
{
  formatTime();
  stream_
    << StringArg(currentThread::tidString(), currentThread::tidStringLength())
    << StringArg(LogLevelName[level], 6)
    << ' ';
}

Logger::~Logger()
{
  if (logLevel_ >= level_) {
    stream_ << " - " << basename_ << ':' << line_ << '\n';
    const LogStream::Buffer &buf(stream_.buffer());
    asyncOutput_->append(buf.data(), buf.length());
    if (UNLIKELY(level_ == FATAL)) {
      asyncOutput_->stop();
      ::abort();
    }
  }
}

LogStream &Logger::stream()
{
  return stream_;
}

bool Logger::init(
  const std::string &basename,
  LogLevel logLevel,
  size_t bucketSize,
  size_t rollSize,
  int flushInterval)
{
  currentThread::tid(); // other created Thread will cache tid in run.
  if (asyncOutput_) { // not the first time call !
    asyncOutput_->stop();
    delete asyncOutput_;
  }
  logLevel_ = logLevel;
  asyncOutput_ =
    new AsyncLogging(basename, bucketSize, rollSize, flushInterval);
  asyncOutput_->start();
}

StringArg Logger::getBasename(StringArg filename)
{
  const char *slash = ::strrchr(filename.data(), '/');
  if (slash) {
    return {slash + 1, filename.size() - (slash - filename.data() + 1)};
  }
  return filename;
}

__thread char t_errnobuf[512];
__thread char t_time[64];
__thread time_t t_lastSecond;

void Logger::formatTime()
{
  int64_t microSecondsSinceEpoch = time_.microSecondsSinceEpoch();
  time_t seconds = static_cast<time_t>(
    microSecondsSinceEpoch / Timestamp::kMicroSecondsPerSecond);
  int microseconds = static_cast<int>(
    microSecondsSinceEpoch % Timestamp::kMicroSecondsPerSecond);
  if (seconds != t_lastSecond) {
    t_lastSecond = seconds;
    struct tm tm_time;
    ::localtime_r(&seconds, &tm_time); // local time
    int len = snprintf(
      t_time, sizeof(t_time), "%4d.%02d.%02d %02d:%02d:%02d",
      tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
      tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
    assert(len == 19);
  }
  char buf[32];
  int length = snprintf(buf, sizeof(buf), ".%06d ", microseconds);
  assert(length == 8);
  stream_ << StringArg(t_time, 19) << StringArg(buf, 8);
}

} // namespace base

} // namespace ycnt
