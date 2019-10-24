//
// Created by jason on 2019/10/22.
//

#include <assert.h>

#include <ycnt/net/Socket.h>
#include <ycnt/net/Acceptor.h>

namespace ycnt
{

namespace net
{

Acceptor::Acceptor(
  EventLoop *loop,
  const InetAddress &listenAddress,
  int maxConnections,
  bool reuseaddr,
  bool reuseport)
  : loop_(loop),
    acceptSocket_(sockets::createNonblockingOrDir(listenAddress.family())),
    acceptChannel_(loop, acceptSocket_.fd()),
    listening_(false),
    maxConnections_(maxConnections)
{
  assert(maxConnections_ >= 0);
  acceptSocket_.setTcpAttr(TcpAttribute::ReuseAddr, reuseaddr);
  acceptSocket_.setTcpAttr(TcpAttribute::ReusePort, reuseport);
  acceptSocket_.bind(listenAddress);
  acceptChannel_.setReadCallback(
    [this](base::Timestamp)
    { this->handleRead(); });
}

Acceptor::~Acceptor()
{
  acceptChannel_.disableAll();
  acceptChannel_.remove();
}

} // namespace net

} // namespace ycnt
