//
// Created by jason on 2019/10/23.
//

#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/tcp.h>

#include <ycnt/net/Socket.h>
#include <ycnt/base/LogStream.h>

namespace ycnt
{

namespace net
{

namespace sockets
{

int createNonblockingOrDir(sa_family_t family)
{
  int socket =
    ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
  if (socket < 0) {
    LOG_FATAL << "::socket error: " << ::strerror(errno);
  }
  return socket;
}

void bindOrDir(int socket, const struct sockaddr *address)
{
  if (::bind(socket, address, sizeof(struct sockaddr_in6)) < 0) {
    LOG_FATAL << "::bind error: " << ::strerror(errno);
  }
}

void listenOrDie(int socket)
{
  if (::listen(socket, SOMAXCONN) < 0) {
    LOG_FATAL << "::listen error: " << ::strerror(errno);
  }
}

int accept(int socket, struct sockaddr_in6 *addr)
{
  socklen_t addrlen = sizeof(struct sockaddr_in6);
  int connfd = ::accept4(
    socket,
    reinterpret_cast<struct sockaddr *>(addr), // FIXME
    &addrlen,
    SOCK_NONBLOCK | SOCK_CLOEXEC);
  if (UNLIKELY(connfd < 0)) {
    int savedErrno = errno;
    switch (savedErrno) {
      // TODO: take notes of these errors
      case EAGAIN:
      case ECONNABORTED:
      case EINTR:
      case EPROTO:
      case EPERM:
      case EMFILE:LOG_WARN << "::accept4 error: " << ::strerror(savedErrno);
        break;
      default:LOG_FATAL << "::accept4 error: " << ::strerror(savedErrno);
        break;
    }
  }
  return connfd;
}

void closeOrDie(int socket)
{
  if (::close(socket) < 0) {
    LOG_FATAL << "::close error: " << ::strerror(errno);
  }
}

void shutdownWriteOrDie(int socket)
{
  if (::shutdown(socket, SHUT_WR) < 0) {
    LOG_FATAL << "::shutdown error: " << ::strerror(errno);
  }
}

int getSocketError(int socket)
{
  int optval;
  socklen_t optlen = sizeof(optval);
  if (::getsockopt(socket, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
    return errno;
  } else {
    return optval;
  }
}

void setSocketOption(int socket, int level, int option, int value)
{
  if (::setsockopt(socket, level, option, &value, sizeof(value)) < 0) {
    LOG_WARN
      << "::setsocketopt level=" << level
      << ", option=" << option
      << ", value=" << value
      << ", error: " << ::strerror(errno);
  }
}

}

Socket::Socket(int fd)
  : fd_(fd)
{
}

Socket::~Socket()
{
  sockets::closeOrDie(fd_);
}

int Socket::fd() const
{
  return fd_;
}

void Socket::bind(const InetAddress &localAddress)
{
  sockets::bindOrDir(fd_, localAddress.getSockAddr());
}

void Socket::listen()
{
  sockets::listenOrDie(fd_);
}

int Socket::accept(InetAddress &remoteAddress)
{
  struct sockaddr_in6 address;
  ::memset(&address, 0, sizeof(address));
  return sockets::accept(fd_, &address);
}

void Socket::shutdownWrite()
{
  sockets::shutdownWriteOrDie(fd_);
}

void Socket::setTcpAttr(TcpAttribute attr, bool on)
{
  switch (attr) {
    case TcpAttribute::KeepAlive:
      sockets::setSocketOption(
        fd_,
        SOL_SOCKET,
        SO_KEEPALIVE,
        on);
      break;
    case TcpAttribute::NoDelay:
      sockets::setSocketOption(
        fd_,
        IPPROTO_TCP,
        TCP_NODELAY,
        on);
      break;
    case TcpAttribute::ReuseAddr:
      sockets::setSocketOption(
        fd_,
        SOL_SOCKET,
        SO_REUSEADDR,
        on);
      break;
    case TcpAttribute::ReusePort:
      sockets::setSocketOption(
        fd_,
        SOL_SOCKET,
        SO_REUSEPORT,
        on);
      break;
    default:LOG_WARN << "unknown tcp attribute: " << attr;
      break;
  }
}

} // namespace net

} // namespace ycnt