//
// Created by jason on 2019/10/7.
//

#ifndef YCNT_YCNT_NET_CHANNEL_H_
#define YCNT_YCNT_NET_CHANNEL_H_

#include <memory>
#include <functional>
#include <poll.h>

#include <ycnt/base/Types.h>
#include <ycnt/base/Timestamp.h>
#include <ycnt/base/LogStream.h>

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

  int fd() const;
  Flag flag() const;
  void setFlag(Flag flag);
  int events() const;
  int revents() const;
  void setRevents(int revents);
  bool isNoneEvent() const;
  void enableReading();
  void disableReading();
  void enableWriting();
  void disableWriting();
  void disableAll();
  bool isReading() const;
  bool isWriting() const;
  void remove();

  // for debugging
  EventLoop *ownerLoop();
  //std::string toString();
  static const char *eventToString(int event);
  static const char *flagToString(Flag flag);
 private:
  DISALLOW_COPY_AND_ASSIGN(Channel);

  static constexpr int kNoneEvent = 0;
  static constexpr int kReadEvent = POLLIN | POLLPRI;
  static constexpr int kWriteEvent = POLLOUT;

  void handleEventImpl(base::Timestamp receiveTime);
  void update();

  EventLoop *loop_;
  const int fd_;
  int events_;
  int revents_;
  Flag flag_;
  std::weak_ptr<void> tie_;
  bool tied_;
  bool eventHandling_;
  bool addedToLoop_;
  ReadCallback readCallback_;
  EventCallback writeCallback_;
  EventCallback closeCallback_;
  EventCallback errorCallback_;
};

base::LogStream &operator<<(base::LogStream &stream, const Channel &channel);

} // namespace net

} // namespace ycnt

#endif //YCNT_YCNT_NET_CHANNEL_H_
