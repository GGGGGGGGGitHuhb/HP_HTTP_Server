#pragma once

#include "base/NonCopyable.h"

#include <sys/epoll.h>
#include <vector>

namespace hp::net {

class Epoller : private hp::base::NonCopyable {
public:
    explicit Epoller(int max_events = 1024);
    ~Epoller();

    void add(int fd, uint32_t events);
    void modify(int fd, uint32_t events);
    void remove(int fd);
    std::vector<epoll_event> wait(int timeout_ms);

private:
    int epoll_fd_{-1};
    std::vector<epoll_event> events_;
};

} // namespace hp::net
