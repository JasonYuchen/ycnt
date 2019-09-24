//
// Created by jason on 2019/9/23.
//

#ifndef YCNT_YCNT_BASE_FILE_H_
#define YCNT_YCNT_BASE_FILE_H_

#include <sys/uio.h>
#include <stdio.h>
#include <string>
#include <memory>

#include <ycnt/base/Types.h>

namespace ycnt
{

namespace base
{

class AppendFile {
 public:
  explicit AppendFile(const std::string &fileName);
  ~AppendFile();
  void append(const char *logline, size_t len);
  void flush();
  size_t writtenBytes() const { return writtenBytes_; }
 private:
  DISALLOW_COPY_AND_ASSIGN(AppendFile);
  size_t write(const char *logline, size_t len);
  FILE *fp_;
  char buffer_[64*1024];
  size_t writtenBytes_;
};

class LogFile {
 public:
  LogFile(const std::string &basename, size_t rollSize, int flushInterval = 3, int checkEveryN = 1024);
  ~LogFile() = default;
  void append(const char *logline, int len); // not thread safe
  void flush();
  bool rollFile();
 private:
  DISALLOW_COPY_AND_ASSIGN(LogFile);
  static std::string getLogFileName(const std::string &basename, time_t *now);
  const std::string basename_;
  const size_t rollSize_;
  const int flushInterval_;
  const int checkEveryN_;
  int count_;
  time_t startOfPeriod_;
  time_t lastRoll_;
  time_t lastFlush_;
  std::unique_ptr<AppendFile> file_;
  const static int kRollPerSeconds_ = 60 * 60 * 24;
};

} // namespace base

} // namespace ycnt

#endif //YCNT_YCNT_BASE_FILE_H_
