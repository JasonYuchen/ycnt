//
// Created by jason on 2019/10/7.
//

#include <assert.h>
#include <memory>

#include <ycnt/net/Channel.h>
#include <ycnt/base/LogStream.h>

using namespace std;

namespace ycnt
{

namespace net
{

Channel::Channel(net::EventLoop *loop, int fd)
  : loop_(loop),
    fd_(fd),
    events_(0),
    revents_(0),
    flag_(kNew),
    tied_(false),
    eventHandling_(false),
    addedToLoop_(false)
{
}

Channel::~Channel()
{
  assert(!eventHandling_);
  assert(!addedToLoop_);
}

void Channel::handleEvent(base::Timestamp receiveTime)
{
  shared_ptr<void> guard;
  if (tied_) {
    guard = tie_.lock();
    if (guard) {
      handleEventImpl(receiveTime);
    }
  } else {
    handleEventImpl(receiveTime);
  }
}

void Channel::handleEventImpl(base::Timestamp receiveTime)
{
  // TODO: why handle these events? take notes...
  eventHandling_ = true;
  if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) {
    LOG_WARN << "POLLHUP: " << toString();
    if (closeCallback_) {
      closeCallback_();
    }
  }

  if (revents_ & POLLNVAL) {
    LOG_WARN << "POLLNVAL: " << toString();
  }

  if (revents_ & (POLLERR | POLLNVAL)) {
    if (errorCallback_) {
      errorCallback_();
    }
  }

  if (revents_ & (POLLIN | POLLPRI | POLLRDHUP)) {
    if (readCallback_) {
      readCallback_(receiveTime);
    }
  }

  if (revents_ & POLLOUT) {
    if (writeCallback_) {
      writeCallback_();
    }
  }
  eventHandling_ = false;
}

void Channel::setReadCallback(ReadCallback cb)
{
  readCallback_ = std::move(cb);
}

void Channel::setWriteCallback(EventCallback cb)
{
  writeCallback_ = std::move(cb);
}

void Channel::setCloseCallback(EventCallback cb)
{
  closeCallback_ = std::move(cb);
}

void Channel::setErrorCallback(EventCallback cb)
{
  errorCallback_ = std::move(cb);
}

int Channel::fd() const
{
  return fd_;
}

Channel::Flag Channel::flag() const
{
  return flag_;
}

void Channel::setFlag(Flag flag)
{
  flag_ = flag;
}

} // net

} // namespace ycnt
