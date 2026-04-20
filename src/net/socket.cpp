#include "net/socket.h"

#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>

#include "net/inet_address.h"

namespace hp::net {
namespace {

std::runtime_error SysError(const std::string& operation) {
  return std::runtime_error(operation + " failed: " + std::strerror(errno));
}

}  // namespace

Socket::Socket() : fd_(::socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0)) {
  if (fd_ < 0) {
    throw SysError("socket");
  }
}

Socket::Socket(int fd) : fd_(fd) {}

Socket::Socket(Socket&& other) noexcept : fd_(other.fd_) {
  other.fd_ = -1;
}

Socket& Socket::operator=(Socket&& other) noexcept {
  if (this != &other) {
    Close();
    fd_ = other.fd_;
    other.fd_ = -1;
  }
  return *this;
}

Socket::~Socket() {
  Close();
}

int Socket::Release() {
  const int old = fd_;
  fd_ = -1;
  return old;
}

void Socket::SetReuseAddr(bool enabled) {
  const int opt = enabled ? 1 : 0;
  if (::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    throw SysError("setsockopt SO_REUSEADDR");
  }
}

void Socket::SetNonBlocking() {
  SetNonBlocking(fd_);
}

void Socket::Bind(const InetAddress& address) {
  const auto& addr = address.Sockaddr();
  if (::bind(fd_, reinterpret_cast<const sockaddr*>(&addr), address.Length()) <
      0) {
    throw SysError("bind");
  }
}

void Socket::Listen(int backlog) {
  if (::listen(fd_, backlog) < 0) {
    throw SysError("listen");
  }
}

int Socket::Accept(InetAddress* peer_address) const {
  sockaddr_in peer{};
  socklen_t len = sizeof(peer);
  const int conn_fd = ::accept4(fd_, reinterpret_cast<sockaddr*>(&peer), &len,
                                SOCK_NONBLOCK | SOCK_CLOEXEC);
  if (conn_fd >= 0 && peer_address != nullptr) {
    peer_address->Sockaddr() = peer;
  }
  return conn_fd;
}

void Socket::Close() {
  if (fd_ >= 0) {
    ::close(fd_);
    fd_ = -1;
  }
}

void Socket::SetNonBlocking(int fd) {
  const int flags = ::fcntl(fd, F_GETFL, 0);
  if (flags < 0) {
    throw SysError("fcntl F_GETFL");
  }
  if (::fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
    throw SysError("fcntl F_SETFL O_NONBLOCK");
  }
}

}  // namespace hp::net
