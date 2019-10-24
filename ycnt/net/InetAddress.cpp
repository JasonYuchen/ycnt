//
// Created by jason on 2019/10/24.
//

#include <string.h>
#include <netdb.h>
#include <assert.h>

#include <ycnt/net/InetAddress.h>
#include <ycnt/base/LogStream.h>

namespace ycnt
{

namespace net
{

InetAddress::InetAddress(uint16_t port, bool loopback, bool ipv6)
{
  if (ipv6) {
    ::memset(&addr6_, 0, sizeof(addr6_));
    addr6_.sin6_family = AF_INET6;
    addr6_.sin6_addr = loopback ? in6addr_loopback : in6addr_any;
    addr6_.sin6_port = ::htons(port);
  } else {
    ::memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = ::htonl(loopback ? INADDR_ANY : INADDR_LOOPBACK);
    addr_.sin_port = ::htons(port);
  }
}

static __thread char t_resolveBuffer[64 * 1024];

// FIXME: use c-ares for async name resolution
bool InetAddress::resolve(StringArg hostname, InetAddress &result)
{
  struct hostent hent;
  struct hostent *he = nullptr;
  int herrno = 0;
  ::memset(&hent, 0, sizeof(hent));
  int ret = ::gethostbyname_r(
    hostname.data(),
    &hent,
    t_resolveBuffer,
    sizeof(t_resolveBuffer),
    &he,
    &herrno);
  if (ret == 0 && he) {
    assert(he->h_addrtype == AF_INET && he->h_length == sizeof(uint32_t));
    result.addr_.sin_addr = *reinterpret_cast<struct in_addr *>(he->h_addr);
    return true;
  } else {
    LOG_WARN << "::gethostbyname_r herron: " << herrno;
    return false;
  }
}

} // namespace net

} // namespace ycnt