//
// Created by jason on 2019/9/23.
//

#include <assert.h>
#include <unistd.h>

#include <ycnt/base/File.h>

using namespace std;

namespace ycnt
{

namespace base
{

AppendFile::AppendFile(const string &filename)
  : fp_(::fopen(filename.c_str(), "ae")),
    writtenBytes_(0)
{
  assert(fp_);
  ::setbuffer(fp_, buffer_, sizeof(buffer_));
}

AppendFile::~AppendFile()
{
  ::fclose(fp_);
}

void AppendFile::append(const char *logline, size_t len)
{
  size_t n = write(logline, len);
  size_t remain = len - n;
  while (remain > 0) {
    size_t x = write(logline + n, remain);
    if (x == 0) {
      int err = ferror(fp_);
      if (err) {
        fprintf(stderr, "AppendFile::append() failed %d\n", err);
      }
      break;
    }
    n += x;
    remain = len - n;
  }
  writtenBytes_ += len;
}

void AppendFile::flush()
{
  ::fflush(fp_);
}

size_t AppendFile::write(const char *logline, size_t len)
{
  return ::fwrite_unlocked(logline, 1, len, fp_);
}

LogFile::LogFile(const string &basename, size_t rollSize, int flushInterval, int checkEveryN)
  : basename_(basename),
    rollSize_(rollSize),
    flushInterval_(flushInterval),
    checkEveryN_(checkEveryN),
    count_(0),
    startOfPeriod_(0),
    lastRoll_(0),
    lastFlush_(0)
{
  rollFile();
}

void LogFile::append(const char *logline, int len)
{
  file_->append(logline, len);
  if (file_->writtenBytes() > rollSize_) {
    rollFile();
  } else {
    ++count_;
    if (count_ >= checkEveryN_) {
      count_ = 0;
      time_t now = ::time(nullptr);
      time_t thisPeriod_ = now / kRollPerSeconds_ * kRollPerSeconds_;
      if (thisPeriod_ != startOfPeriod_) {
        rollFile();
      } else if (now - lastFlush_ > flushInterval_) {
        lastFlush_ = now;
        file_->flush();
      }
    }
  }
}

void LogFile::flush()
{
  file_->flush();
}

bool LogFile::rollFile()
{
  time_t now = 0;
  string filename = getLogFileName(basename_, &now);
  time_t start = now / kRollPerSeconds_ * kRollPerSeconds_;
  if (now > lastRoll_) {
    lastRoll_ = now;
    lastFlush_ = now;
    startOfPeriod_ = start;
    file_.reset(new AppendFile(filename));
    return true;
  }
  return false;
}

std::string LogFile::getLogFileName(const std::string &basename, time_t *now)
{
  string filename;
  filename.reserve(basename.size() + 64);
  filename = basename;
  char timebuf[32];
  struct tm tm;
  *now = ::time(nullptr);
  gmtime_r(now, &tm);
  strftime(timebuf, sizeof(timebuf), ".%Y%m%d-%H%M%S.", &tm);
  filename += timebuf;

  char hostname[256];
  if(::gethostname(hostname, sizeof(hostname)) == 0) {
    hostname[sizeof(hostname) - 1] = '\0';
  } else {
    snprintf(hostname, sizeof(hostname), "unknownhost");
  }
  filename += hostname;

  char pidbuf[32];
  snprintf(pidbuf, sizeof(pidbuf), ".%d", ::getpid());
  filename += pidbuf;

  filename += ".log";

  return filename;
}

} // namespace base

} // namespace ycnt
