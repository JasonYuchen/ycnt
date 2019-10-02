//
// Created by jason on 2019/10/2.
//

#ifndef YCNT_YCNT_NET_EVENTLOOP_H_
#define YCNT_YCNT_NET_EVENTLOOP_H_

#include <ycnt/base/Types.h>

namespace ycnt
{

namespace net
{

class EventLoop {
 public:
  EventLoop();
  ~EventLoop();
  void loop();
  void assertInLoopThread();
 private:
  void abortNotInLoopThread();
  DISALLOW_COPY_AND_ASSIGN(EventLoop);
};

} // namespace net

} // namespace ycnt

#endif //YCNT_YCNT_NET_EVENTLOOP_H_
