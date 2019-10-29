//
// Created by jason on 2019/10/22.
//

#include <assert.h>

#include <ycnt/net/Socket.h>
#include <ycnt/net/Acceptor.h>

using namespace std;

namespace ycnt
{

namespace net
{

Acceptor::Acceptor(
  EventLoop *loop,
  const InetAddress &listenAddress,
  bool reuseaddr,
  bool reuseport)
  : loop_(loop),
    acceptSocket_(sockets::createNonblockingOrDir(listenAddress.family())),
    acceptChannel_(loop, acceptSocket_.fd()),
    listening_(false),
    pause_(false)
{
  acceptSocket_.setTcpAttr(TcpAttribute::ReuseAddr, reuseaddr);
  acceptSocket_.setTcpAttr(TcpAttribute::ReusePort, reuseport);
  acceptSocket_.bind(listenAddress);
  acceptChannel_.setReadCallback(
    [this](base::Timestamp now)
    { this->handleRead(); });
}

Acceptor::~Acceptor()
{
  acceptChannel_.disableAll();
  acceptChannel_.remove();
}

void Acceptor::setNewConnectionCallback(const NewConnectionCallback &cb)
{
  newConnectionCallback_ = cb;
}

bool Acceptor::listening() const
{
  return listening_;
}

void Acceptor::listen()
{
  loop_->assertInLoopThread();
  listening_ = true;
  acceptSocket_.listen();
  acceptChannel_.enableReading();
}

void Acceptor::pause()
{
  loop_->assertInLoopThread();
  pause_ = true;
}

void Acceptor::resume()
{
  loop_->assertInLoopThread();
  pause_ = false;
}

void Acceptor::handleRead()
{
  loop_->assertInLoopThread();
  InetAddress remoteAddress;
  int connfd = acceptSocket_.accept(remoteAddress);
  if (connfd >= 0) {
    if (newConnectionCallback_ && !pause_) {
      newConnectionCallback_(connfd, remoteAddress);
    } else {
      ::close(connfd);
    }
  } else {
    if (pause_) {
      LOG_WARN << "Acceptor is paused";
    }
  }
}

} // namespace net

} // namespace ycnt
