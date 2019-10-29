//
// Created by jason on 2019/10/24.
//

#ifndef YCNT_YCNT_NET_INETADDRESS_H_
#define YCNT_YCNT_NET_INETADDRESS_H_

#include <netinet/in.h>
#include <string>
#include <vector>
#include <variant>

#include <ycnt/base/Types.h>

namespace ycnt
{

namespace net
{

class InetAddress {
 public:
  DEFAULT_COPY_AND_ASSIGN(InetAddress);
  explicit InetAddress(
    uint16_t port = 0,
    bool loopback = false,
    bool ipv6 = false);
  explicit InetAddress(std::string_view ip, uint16_t port, bool ipv6 = false);
  explicit InetAddress(const struct sockaddr_in &addr);
  explicit InetAddress(const struct sockaddr_in6 &addr);
  sa_family_t family() const;
  std::string toIp() const;
  std::string toIpPort() const;
  uint16_t toPort() const;
  const struct sockaddr *getSockAddr() const;
  void setSockAddrInet6(const struct sockaddr_in6 &addr6);
  void setSockAddr(const struct sockaddr_in &addr);

  static bool resolve(std::string_view hostname, InetAddress &result);
  static bool resolveAll(
    std::string_view hostname,
    std::vector<InetAddress> &results);

 private:
  union {
    struct sockaddr_in addr_;
    struct sockaddr_in6 addr6_;
  };
};

} // namespace net

} // namespace ycnt

#endif //YCNT_YCNT_NET_INETADDRESS_H_
