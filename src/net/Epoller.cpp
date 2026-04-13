#include "net/Epoller.h"

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>
#include <sys/epoll.h>
#include <unistd.h>

namespace hp::net {
namespace {

std::runtime_error sysError(const std::string& operation) {
    return std::runtime_error(operation + " failed: " + std::strerror(errno));
}

} // namespace

Epoller::Epoller(int max_events) : epoll_fd_(::epoll_create1(EPOLL_CLOEXEC)), events_(max_events) {
    if (epoll_fd_ < 0) {
        throw sysError("epoll_create1");
    }
}

Epoller::~Epoller() {
    if (epoll_fd_ >= 0) {
        ::close(epoll_fd_);
    }
}

void Epoller::add(int fd, uint32_t events) {
    epoll_event event{};
    event.events = events;
    event.data.fd = fd;
    if (::epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &event) < 0) {
        throw sysError("epoll_ctl ADD");
    }
}

void Epoller::modify(int fd, uint32_t events) {
    epoll_event event{};
    event.events = events;
    event.data.fd = fd;
    if (::epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &event) < 0) {
        throw sysError("epoll_ctl MOD");
    }
}

void Epoller::remove(int fd) {
    if (::epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr) < 0 && errno != EBADF && errno != ENOENT) {
        throw sysError("epoll_ctl DEL");
    }
}

std::vector<epoll_event> Epoller::wait(int timeout_ms) {
    while (true) {
        const int n = ::epoll_wait(epoll_fd_, events_.data(), static_cast<int>(events_.size()), timeout_ms);
        if (n >= 0) {
            return {events_.begin(), events_.begin() + n};
        }
        if (errno == EINTR) {
            continue;
        }
        throw sysError("epoll_wait");
    }
}

} // namespace hp::net
