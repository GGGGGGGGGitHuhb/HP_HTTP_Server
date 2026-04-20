#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

#include "base/non_copyable.h"
#include "http/http_server.h"
#include "net/epoller.h"
#include "net/socket.h"

namespace hp::net {

/**
 * @brief M1 单线程非阻塞 epoll TCP/HTTP 服务器。
 *
 * TcpServer 负责监听 socket、客户端连接状态和 epoll 事件循环；HTTP 语义交给
 * http::HttpServer。 M1 每个请求响应后关闭连接，不实现 keep-alive、线程池或主从
 * Reactor。
 */
class TcpServer : private hp::base::NonCopyable {
 public:
  /**
   * @brief 创建服务器实例。
   *
   * @param port 监听端口。
   * @param static_root 静态文件根目录。
   */
  TcpServer(uint16_t port, std::string static_root);

  /**
   * @brief 启动事件循环。
   *
   * 该调用会阻塞运行，直到进程退出；优雅关闭留到后续里程碑实现。
   */
  void Start();

 private:
  /**
   * @brief 单个客户端连接的最小状态。
   *
   * read_buffer 保存可能分片到达的请求头；write_buffer 与 write_offset
   * 用于处理非阻塞短写。
   */
  struct Connection {
    int fd_{-1};
    std::string read_buffer_;
    std::string write_buffer_;
    std::size_t write_offset_{0};
    bool response_ready_{false};
  };

  void SetupListenSocket();
  void HandleListenEvent();
  void HandleReadEvent(int fd);
  void HandleWriteEvent(int fd);
  void CloseConnection(int fd);
  void UpdateInterest(int fd, uint32_t events);

  // M1 固定上限：避免异常客户端无限堆积内存或事件数组。
  static constexpr std::size_t kReadBufferSize = 4096;
  static constexpr std::size_t kMaxRequestHeaderSize = 16 * 1024;
  static constexpr int kListenBacklog = 1024;
  static constexpr int kEpollTimeoutMs = 1000;

  uint16_t port_;
  Socket listen_socket_;
  Epoller epoller_;
  hp::http::HttpServer http_server_;
  std::unordered_map<int, Connection> connections_;
};

}  // namespace hp::net
