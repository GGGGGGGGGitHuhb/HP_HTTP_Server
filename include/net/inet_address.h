#pragma once

#include <netinet/in.h>

#include <string>

namespace hp::net {

/**
 * @brief IPv4 地址与端口的轻量封装。
 *
 * M1 只需要 IPv4 listen 地址，因此内部保存 sockaddr_in，供 bind/accept 等
 * socket API 使用。
 */
class InetAddress {
 public:
  /**
   * @brief 构造一个 IPv4 地址。
   *
   * @param port 主机字节序端口。
   * @param ip 点分十进制 IPv4 地址，默认监听所有网卡。
   */
  explicit InetAddress(uint16_t port, std::string ip = "0.0.0.0");

  const sockaddr_in& Sockaddr() const {
    return addr_;
  }
  sockaddr_in& Sockaddr() {
    return addr_;
  }
  socklen_t Length() const {
    return sizeof(addr_);
  }

  /**
   * @brief 返回便于日志展示的 host:port 字符串。
   */
  std::string ToHostPort() const;

 private:
  sockaddr_in addr_{};
};

}  // namespace hp::net
