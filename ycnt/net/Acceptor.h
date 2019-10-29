//
// Created by jason on 2019/10/22.
//

#ifndef YCNT_YCNT_NET_ACCEPTOR_H_
#define YCNT_YCNT_NET_ACCEPTOR_H_

#include <functional>

#include <ycnt/net/Socket.h>
#include <ycnt/net/EventLoop.h>
#include <ycnt/net/Channel.h>
#include <ycnt/net/InetAddress.h>
#include <ycnt/base/Types.h>

namespace ycnt
{

namespace net
{

class Acceptor {
 public:
  using NewConnectionCallback = std::function<void(
    int socket,
    const InetAddress &)>;
  Acceptor(
    EventLoop *loop,
    const InetAddress &listenAddress,
    bool reuseaddr,
    bool reuseport);
  ~Acceptor();
  void setNewConnectionCallback(const NewConnectionCallback &cb);
  bool listening() const;
  void listen();
  void pause();
  void resume();
 private:
  DISALLOW_COPY_AND_ASSIGN(Acceptor);
  void handleRead();

  EventLoop *loop_;
  Socket acceptSocket_;
  Channel acceptChannel_;
  NewConnectionCallback newConnectionCallback_;
  bool listening_;
  bool pause_;
};

} // namespace net

} // namespace ycnt

#endif //YCNT_YCNT_NET_ACCEPTOR_H_
