//
// Created by jason on 2019/10/7.
//

#ifndef YCNT_YCNT_NET_CHANNEL_H_
#define YCNT_YCNT_NET_CHANNEL_H_

#include <memory>
#include <functional>

#include <ycnt/base/Types.h>
#include <ycnt/base/Timestamp.h>

namespace ycnt
{

namespace net
{

class EventLoop;

class Channel {
 public:
  enum Flag { kNew, kAdded, kDeleted, NUM_OF_FLAGS };
  using EventCallback = std::function<void()>;
  using ReadCallback = std::function<void(base::Timestamp)>;

  Channel(EventLoop *loop, int fd);
  ~Channel();

  void handleEvent(base::Timestamp receiveTime);
  void setReadCallback(ReadCallback cb);
  void setWriteCallback(EventCallback cb);
  void setCloseCallback(EventCallback cb);
  void setErrorCallback(EventCallback cb);

  int fd() const
  { throw "unimplemented"; }
  Flag flag() const
  { throw "unimplemented"; }
  void setFlag(Flag flag) const;
  int events() const;
  void setRevents(int revents);
  bool isNoneEvent() const;
  void enableReading();
  void disableReading();
  void enableWriting();
  void disableWriting();
  void disableAll();

  // for debugging
  std::string toString();
  static const char *eventToString();
  static const char *flagToString();
 private:
  DISALLOW_COPY_AND_ASSIGN(Channel);
  EventLoop *loop_;
  const int fd_;
  int events_;
  int revents_;
  Flag flag_;
  std::weak_ptr<void> tie_;
  bool tied_;
  bool eventHandling_;
  ReadCallback readCallback_;
  EventCallback writeCallback_;
  EventCallback closeCallback_;
  EventCallback errorCallback_;
};

} // namespace net

} // namespace ycnt

#endif //YCNT_YCNT_NET_CHANNEL_H_
