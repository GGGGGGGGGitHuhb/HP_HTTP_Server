#pragma once

#include "base/NonCopyable.h"
#include "net/InetAddress.h"

namespace hp::net {

class Socket : private hp::base::NonCopyable {
public:
    Socket();
    explicit Socket(int fd);
    Socket(Socket&& other) noexcept;
    Socket& operator=(Socket&& other) noexcept;
    ~Socket();

    int fd() const { return fd_; }
    int release();

    void setReuseAddr(bool enabled);
    void setNonBlocking();
    void bind(const InetAddress& address);
    void listen(int backlog = 1024);
    int accept(InetAddress* peer_address = nullptr) const;
    void close();

    static void setNonBlocking(int fd);

private:
    int fd_{-1};
};

} // namespace hp::net
