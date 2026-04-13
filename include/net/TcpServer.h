#pragma once

#include "base/NonCopyable.h"
#include "http/HttpServer.h"
#include "net/Epoller.h"
#include "net/Socket.h"

#include <cstdint>
#include <string>
#include <unordered_map>

namespace hp::net {

class TcpServer : private hp::base::NonCopyable {
public:
    TcpServer(uint16_t port, std::string static_root); // 创建实例

    void start(); // 启动 TcpServer

private:
    struct Connection {
        int fd{-1};
        std::string read_buffer; // 从客户端读到的数据
        std::string write_buffer; // 准备写回客户端的数据
        std::size_t write_offset{0}; // 已经写到的位置
        bool response_ready{false}; // 是否已经生成响应
        // 请求可能分多次到达，响应可能分多次写完
    };

    void setupListenSocket();
    void handleListenEvent();
    void handleReadEvent(int fd);
    void handleWriteEvent(int fd);
    void closeConnection(int fd);
    void updateInterest(int fd, uint32_t events);

    // 类中常量使用 static constexpr
    static constexpr std::size_t kReadBufferSize = 4096;
    static constexpr std::size_t kMaxRequestHeaderSize = 16 * 1024;
    static constexpr int kListenBacklog = 1024;
    static constexpr int kEpollTimeoutMs = 1000;

    uint16_t port_;
    Socket listen_socket_;
    Epoller epoller_; // 事件监听器，看哪些连接可 IO
    hp::http::HttpServer http_server_; // HTTP 处理器，收到请求后给它
    std::unordered_map<int, Connection> connections_; // 当前所有客户端连接
};

} // namespace hp::net
