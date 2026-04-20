#pragma once

#include <sys/epoll.h>

#include <vector>

#include "base/non_copyable.h"

namespace hp::net {

/**
 * @brief epoll fd 的 RAII 封装。
 *
 * M1 使用 LT 模式，Epoller 只负责
 * epoll_create/ctl/wait，不决定业务事件如何处理。
 */
class Epoller : private hp::base::NonCopyable {
 public:
  /**
   * @brief 创建 epoll 实例并预分配事件数组。
   *
   * @param max_events 单次 epoll_wait 最多返回的事件数量。
   */
  explicit Epoller(int max_events = 1024);

  /**
   * @brief 关闭 epoll fd。
   */
  ~Epoller();

  /**
   * @brief 添加 fd 监听事件。
   */
  void Add(int fd, uint32_t events);

  /**
   * @brief 修改 fd 监听事件。
   */
  void Modify(int fd, uint32_t events);

  /**
   * @brief 从 epoll 中移除 fd。
   */
  void Remove(int fd);

  /**
   * @brief 等待就绪事件。
   *
   * @param timeout_ms 等待超时时间，单位毫秒。
   * @return 本轮就绪事件列表。
   */
  std::vector<epoll_event> Wait(int timeout_ms);

 private:
  int epoll_fd_{-1};
  std::vector<epoll_event> events_;
};

}  // namespace hp::net
