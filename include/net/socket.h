#pragma once

#include "base/non_copyable.h"

namespace hp::net {

class InetAddress;

/**
 * @brief Linux TCP socket 的 RAII 封装。
 *
 * Socket 独占一个 fd，析构时自动关闭；移动后所有权转移，拷贝被禁止。
 */
class Socket : private hp::base::NonCopyable {
 public:
  /**
   * @brief 创建一个 TCP socket。
   */
  Socket();

  /**
   * @brief 接管一个已有 fd 的所有权。
   */
  explicit Socket(int fd);

  Socket(Socket&& other) noexcept;
  Socket& operator=(Socket&& other) noexcept;

  /**
   * @brief 关闭持有的 fd。
   */
  ~Socket();

  int Fd() const {
    return fd_;
  }

  /**
   * @brief 释放 fd 所有权，调用方负责后续关闭。
   */
  int Release();

  /**
   * @brief 设置 SO_REUSEADDR。
   */
  void SetReuseAddr(bool enabled);

  /**
   * @brief 将当前 socket 设置为非阻塞。
   */
  void SetNonBlocking();

  /**
   * @brief 将 socket 绑定到指定地址。
   */
  void Bind(const InetAddress& address);

  /**
   * @brief 开始监听连接。
   *
   * @param backlog listen 队列长度。
   */
  void Listen(int backlog = 1024);

  /**
   * @brief 接收一个客户端连接。
   *
   * 返回的 client fd 已设置为非阻塞和 close-on-exec；失败时返回 -1，调用方检查
   * errno。
   */
  int Accept(InetAddress* peer_address = nullptr) const;

  /**
   * @brief 显式关闭当前持有的 fd。
   */
  void Close();

  /**
   * @brief 将任意 fd 设置为非阻塞。
   */
  static void SetNonBlocking(int fd);

 private:
  int fd_{-1};
};

}  // namespace hp::net
