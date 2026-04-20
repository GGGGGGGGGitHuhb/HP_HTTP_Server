#pragma once

#include <string_view>

namespace hp::base {

/**
 * @brief M1 同步日志级别。
 *
 * M1 只输出到 stdout/stderr，不做异步队列和落盘策略；后续 M5 再扩展异步日志。
 */
enum class LogLevel {
  kInfo,
  kWarn,
  kError,
};

/**
 * @brief 输出一条指定级别的同步日志。
 *
 * @param level 日志级别。
 * @param message 日志正文，调用方无需包含换行符。
 */
void Log(LogLevel level, std::string_view message);

/**
 * @brief 输出 info 级别日志。
 */
void Info(std::string_view message);

/**
 * @brief 输出 warn 级别日志。
 */
void Warn(std::string_view message);

/**
 * @brief 输出 error 级别日志。
 */
void Error(std::string_view message);

}  // namespace hp::base
