#include "net/Socket.h"

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

namespace hp::net {
namespace {

std::runtime_error sysError(const std::string& operation) {
    return std::runtime_error(operation + " failed: " + std::strerror(errno));
}

} // namespace

Socket::Socket() : fd_(::socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0)) {
    if (fd_ < 0) {
        throw sysError("socket");
    }
}

Socket::Socket(int fd) : fd_(fd) {}

Socket::Socket(Socket&& other) noexcept : fd_(other.fd_) {
    other.fd_ = -1;
}

Socket& Socket::operator=(Socket&& other) noexcept {
    if (this != &other) {
        close();
        fd_ = other.fd_;
        other.fd_ = -1;
    }
    return *this;
}

Socket::~Socket() {
    close();
}

int Socket::release() {
    const int old = fd_;
    fd_ = -1;
    return old;
}

void Socket::setReuseAddr(bool enabled) {
    const int opt = enabled ? 1 : 0;
    if (::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        throw sysError("setsockopt SO_REUSEADDR");
    }
}

void Socket::setNonBlocking() {
    setNonBlocking(fd_);
}

void Socket::bind(const InetAddress& address) {
    const auto& addr = address.sockaddr();
    if (::bind(fd_, reinterpret_cast<const sockaddr*>(&addr), address.length()) < 0) {
        throw sysError("bind");
    }
}

void Socket::listen(int backlog) {
    if (::listen(fd_, backlog) < 0) {
        throw sysError("listen");
    }
}

int Socket::accept(InetAddress* peer_address) const {
    sockaddr_in peer{};
    socklen_t len = sizeof(peer);
    const int conn_fd = ::accept4(fd_, reinterpret_cast<sockaddr*>(&peer), &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (conn_fd >= 0 && peer_address != nullptr) {
        peer_address->sockaddr() = peer;
    }
    return conn_fd;
}

void Socket::close() {
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
}

void Socket::setNonBlocking(int fd) {
    const int flags = ::fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        throw sysError("fcntl F_GETFL");
    }
    if (::fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        throw sysError("fcntl F_SETFL O_NONBLOCK");
    }
}

} // namespace hp::net
