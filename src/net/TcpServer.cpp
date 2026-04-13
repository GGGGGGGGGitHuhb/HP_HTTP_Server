#include "net/TcpServer.h"

#include "base/Logger.h"
#include "net/InetAddress.h"

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

namespace hp::net {
namespace {

bool wouldBlock() {
    return errno == EAGAIN || errno == EWOULDBLOCK;
}

std::string errnoMessage(const std::string& operation) {
    return operation + " failed: " + std::strerror(errno);
}

} // namespace

TcpServer::TcpServer(uint16_t port, std::string static_root)
    : port_(port), listen_socket_(), epoller_(1024), http_server_(std::move(static_root)) {}

void TcpServer::start() {
    setupListenSocket();
    hp::base::info("server listening on 0.0.0.0:" + std::to_string(port_));

    while (true) {
        const auto events = epoller_.wait(kEpollTimeoutMs);
        for (const auto& event : events) {
            const int fd = event.data.fd;
            if (fd == listen_socket_.fd()) {
                handleListenEvent();
                continue;
            }

            if ((event.events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) != 0U) {
                closeConnection(fd);
                continue;
            }
            if ((event.events & EPOLLIN) != 0U) {
                handleReadEvent(fd);
            }
            if (connections_.find(fd) != connections_.end() && (event.events & EPOLLOUT) != 0U) {
                handleWriteEvent(fd);
            }
        }
    }
}

void TcpServer::setupListenSocket() {
    listen_socket_.setReuseAddr(true);
    listen_socket_.setNonBlocking();
    const InetAddress listen_addr(port_);
    listen_socket_.bind(listen_addr);
    listen_socket_.listen(kListenBacklog);
    epoller_.add(listen_socket_.fd(), EPOLLIN);
}

void TcpServer::handleListenEvent() {
    while (true) {
        const int client_fd = listen_socket_.accept(nullptr);
        if (client_fd >= 0) {
            Connection connection;
            connection.fd = client_fd;
            connection.read_buffer.reserve(kReadBufferSize);
            connections_.emplace(client_fd, std::move(connection));
            epoller_.add(client_fd, EPOLLIN | EPOLLRDHUP);
            continue;
        }
        if (wouldBlock()) {
            return;
        }
        if (errno == EINTR) {
            continue;
        }
        hp::base::warn(errnoMessage("accept"));
        return;
    }
}

void TcpServer::handleReadEvent(int fd) {
    auto it = connections_.find(fd);
    if (it == connections_.end()) {
        return;
    }
    auto& connection = it->second;

    char buffer[kReadBufferSize];
    while (true) {
        const ssize_t n = ::read(fd, buffer, sizeof(buffer));
        if (n > 0) {
            connection.read_buffer.append(buffer, static_cast<std::size_t>(n));
            if (connection.read_buffer.size() > kMaxRequestHeaderSize) {
                connection.write_buffer = http_server_.handleRequest("BAD_REQUEST_TOO_LARGE\r\n\r\n");
                connection.write_offset = 0;
                connection.response_ready = true;
                updateInterest(fd, EPOLLOUT | EPOLLRDHUP);
                return;
            }
            if (connection.read_buffer.find("\r\n\r\n") != std::string::npos) {
                connection.write_buffer = http_server_.handleRequest(connection.read_buffer);
                connection.write_offset = 0;
                connection.response_ready = true;
                updateInterest(fd, EPOLLOUT | EPOLLRDHUP);
                return;
            }
            continue;
        }
        if (n == 0) {
            closeConnection(fd);
            return;
        }
        if (errno == EINTR) {
            continue;
        }
        if (wouldBlock()) {
            return;
        }
        hp::base::warn(errnoMessage("read"));
        closeConnection(fd);
        return;
    }
}

void TcpServer::handleWriteEvent(int fd) {
    auto it = connections_.find(fd);
    if (it == connections_.end()) {
        return;
    }
    auto& connection = it->second;

    while (connection.write_offset < connection.write_buffer.size()) {
        const char* data = connection.write_buffer.data() + connection.write_offset;
        const std::size_t remaining = connection.write_buffer.size() - connection.write_offset;
        const ssize_t n = ::write(fd, data, remaining);
        if (n > 0) {
            connection.write_offset += static_cast<std::size_t>(n);
            continue;
        }
        if (n == 0) {
            return;
        }
        if (errno == EINTR) {
            continue;
        }
        if (wouldBlock()) {
            updateInterest(fd, EPOLLOUT | EPOLLRDHUP);
            return;
        }
        hp::base::warn(errnoMessage("write"));
        closeConnection(fd);
        return;
    }

    closeConnection(fd);
}

void TcpServer::closeConnection(int fd) {
    if (connections_.erase(fd) == 0) {
        return;
    }
    try {
        epoller_.remove(fd);
    } catch (const std::exception& ex) {
        hp::base::warn(ex.what());
    }
    ::close(fd);
}

void TcpServer::updateInterest(int fd, uint32_t events) {
    try {
        epoller_.modify(fd, events);
    } catch (const std::exception& ex) {
        hp::base::warn(ex.what());
        closeConnection(fd);
    }
}

} // namespace hp::net
