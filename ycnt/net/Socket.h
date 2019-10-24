//
// Created by jason on 2019/10/23.
//

#ifndef YCNT_YCNT_NET_SOCKET_H_
#define YCNT_YCNT_NET_SOCKET_H_

#include <ycnt/base/Types.h>

namespace ycnt
{

namespace net
{

class InetAddress;

enum TcpAttribute {
  NoDelay, ReuseAddr, ReusePort, KeepAlive, NumOfAttribute
};

class Socket {
 public:
  explicit Socket(int fd);
  ~Socket();
  int fd() const;
  void bind(const InetAddress &localAddress);
  void listen();
  int accept(InetAddress &remoteAddress);
  void shutdownWrite();
  void setTcpAttr(TcpAttribute attr, bool on);
 private:
  DISALLOW_COPY_AND_ASSIGN(Socket);
  const int fd_;
};

}

} // namespace ycnt

#endif //YCNT_YCNT_NET_SOCKET_H_
