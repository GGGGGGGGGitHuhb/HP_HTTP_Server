#include "net/InetAddress.h"

#include <arpa/inet.h>
#include <stdexcept>

namespace hp::net {

InetAddress::InetAddress(uint16_t port, std::string ip) {
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    if (inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr) != 1) {
        throw std::runtime_error("invalid IPv4 address: " + ip);
    }
}

std::string InetAddress::toHostPort() const {
    char buffer[INET_ADDRSTRLEN]{};
    inet_ntop(AF_INET, &addr_.sin_addr, buffer, sizeof(buffer));
    return std::string(buffer) + ":" + std::to_string(ntohs(addr_.sin_port));
}

} // namespace hp::net
