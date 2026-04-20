#include "net/tcp_server.h"

#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>

#include "base/logger.h"
#include "net/inet_address.h"

namespace hp::net {
namespace {

bool WouldBlock() {
  return errno == EAGAIN || errno == EWOULDBLOCK;
}

std::string ErrnoMessage(const std::string& operation) {
  return operation + " failed: " + std::strerror(errno);
}

}  // namespace

TcpServer::TcpServer(uint16_t port, std::string static_root)
    : port_(port),
      listen_socket_(),
      epoller_(1024),
      http_server_(std::move(static_root)) {}

void TcpServer::Start() {
  SetupListenSocket();
  hp::base::Info("server listening on 0.0.0.0:" + std::to_string(port_));

  while (true) {
    const auto events = epoller_.Wait(kEpollTimeoutMs);
    for (const auto& event : events) {
      const int fd = event.data.fd;
      if (fd == listen_socket_.Fd()) {
        HandleListenEvent();
        continue;
      }

      if ((event.events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) != 0U) {
        CloseConnection(fd);
        continue;
      }
      if ((event.events & EPOLLIN) != 0U) {
        HandleReadEvent(fd);
      }
      if (connections_.find(fd) != connections_.end() &&
          (event.events & EPOLLOUT) != 0U) {
        HandleWriteEvent(fd);
      }
    }
  }
}

void TcpServer::SetupListenSocket() {
  listen_socket_.SetReuseAddr(true);
  listen_socket_.SetNonBlocking();
  const InetAddress listen_addr(port_);
  listen_socket_.Bind(listen_addr);
  listen_socket_.Listen(kListenBacklog);
  epoller_.Add(listen_socket_.Fd(), EPOLLIN);
}

void TcpServer::HandleListenEvent() {
  while (true) {
    const int client_fd = listen_socket_.Accept(nullptr);
    if (client_fd >= 0) {
      Connection connection;
      connection.fd_ = client_fd;
      connection.read_buffer_.reserve(kReadBufferSize);
      connections_.emplace(client_fd, std::move(connection));
      epoller_.Add(client_fd, EPOLLIN | EPOLLRDHUP);
      continue;
    }
    if (WouldBlock()) {
      return;
    }
    if (errno == EINTR) {
      continue;
    }
    hp::base::Warn(ErrnoMessage("accept"));
    return;
  }
}

void TcpServer::HandleReadEvent(int fd) {
  auto it = connections_.find(fd);
  if (it == connections_.end()) {
    return;
  }
  auto& connection = it->second;

  char buffer[kReadBufferSize];
  while (true) {
    const ssize_t n = ::read(fd, buffer, sizeof(buffer));
    if (n > 0) {
      connection.read_buffer_.append(buffer, static_cast<std::size_t>(n));
      if (connection.read_buffer_.size() > kMaxRequestHeaderSize) {
        connection.write_buffer_ =
            http_server_.HandleRequest("BAD_REQUEST_TOO_LARGE\r\n\r\n");
        connection.write_offset_ = 0;
        connection.response_ready_ = true;
        UpdateInterest(fd, EPOLLOUT | EPOLLRDHUP);
        return;
      }
      if (connection.read_buffer_.find("\r\n\r\n") != std::string::npos) {
        connection.write_buffer_ =
            http_server_.HandleRequest(connection.read_buffer_);
        connection.write_offset_ = 0;
        connection.response_ready_ = true;
        UpdateInterest(fd, EPOLLOUT | EPOLLRDHUP);
        return;
      }
      continue;
    }
    if (n == 0) {
      CloseConnection(fd);
      return;
    }
    if (errno == EINTR) {
      continue;
    }
    if (WouldBlock()) {
      return;
    }
    hp::base::Warn(ErrnoMessage("read"));
    CloseConnection(fd);
    return;
  }
}

void TcpServer::HandleWriteEvent(int fd) {
  auto it = connections_.find(fd);
  if (it == connections_.end()) {
    return;
  }
  auto& connection = it->second;

  while (connection.write_offset_ < connection.write_buffer_.size()) {
    const char* data =
        connection.write_buffer_.data() + connection.write_offset_;
    const std::size_t remaining =
        connection.write_buffer_.size() - connection.write_offset_;
    const ssize_t n = ::write(fd, data, remaining);
    if (n > 0) {
      connection.write_offset_ += static_cast<std::size_t>(n);
      continue;
    }
    if (n == 0) {
      return;
    }
    if (errno == EINTR) {
      continue;
    }
    if (WouldBlock()) {
      UpdateInterest(fd, EPOLLOUT | EPOLLRDHUP);
      return;
    }
    hp::base::Warn(ErrnoMessage("write"));
    CloseConnection(fd);
    return;
  }

  CloseConnection(fd);
}

void TcpServer::CloseConnection(int fd) {
  if (connections_.erase(fd) == 0) {
    return;
  }
  try {
    epoller_.Remove(fd);
  } catch (const std::exception& ex) {
    hp::base::Warn(ex.what());
  }
  ::close(fd);
}

void TcpServer::UpdateInterest(int fd, uint32_t events) {
  try {
    epoller_.Modify(fd, events);
  } catch (const std::exception& ex) {
    hp::base::Warn(ex.what());
    CloseConnection(fd);
  }
}

}  // namespace hp::net
