#pragma once

namespace hp::base {

/**
 * @brief 禁止派生类拷贝的基类。
 *
 * 用于 fd、epoll 等独占资源包装类，避免多个对象同时管理同一个底层资源。
 */
class NonCopyable {
 protected:
  NonCopyable() = default;
  ~NonCopyable() = default;

 public:
  NonCopyable(const NonCopyable&) = delete;
  NonCopyable& operator=(const NonCopyable&) = delete;
};

}  // namespace hp::base
