//
// Created by jason on 2019/10/7.
//

#ifndef YCNT_YCNT_NET_CHANNEL_H_
#define YCNT_YCNT_NET_CHANNEL_H_

#include <ycnt/base/Types.h>

namespace ycnt
{

namespace net
{

class EventLoop;

class Channel {
 public:
  enum Flag { kNew, kAdded, kDeleted, NUM_OF_FLAGS };
  Channel(EventLoop *loop, int fd);
  int fd() const;
  Flag flag() const;
  void setFlag() const;
  ~Channel();
 private:
  DISALLOW_COPY_AND_ASSIGN(Channel);
};

} // namespace net

} // namespace ycnt

#endif //YCNT_YCNT_NET_CHANNEL_H_
