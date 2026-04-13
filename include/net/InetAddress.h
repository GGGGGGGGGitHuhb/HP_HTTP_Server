#pragma once

#include <netinet/in.h>
#include <string>

namespace hp::net {

class InetAddress {
public:
    explicit InetAddress(uint16_t port, std::string ip = "0.0.0.0");

    const sockaddr_in& sockaddr() const { return addr_; }
    sockaddr_in& sockaddr() { return addr_; }
    socklen_t length() const { return sizeof(addr_); }
    std::string toHostPort() const;

private:
    sockaddr_in addr_{};
};

} // namespace hp::net
