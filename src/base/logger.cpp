#include "base/logger.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>

namespace hp::base {
namespace {

std::mutex log_mutex;

const char* ToString(LogLevel level) {
  switch (level) {
    case LogLevel::kInfo:
      return "INFO";
    case LogLevel::kWarn:
      return "WARN";
    case LogLevel::kError:
      return "ERROR";
  }
  return "UNKNOWN";
}

std::string NowString() {
  const auto now = std::chrono::system_clock::now();
  const std::time_t time = std::chrono::system_clock::to_time_t(now);
  std::tm tm{};
  localtime_r(&time, &tm);

  std::ostringstream oss;
  oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
  return oss.str();
}

}  // namespace

void Log(LogLevel level, std::string_view message) {
  std::lock_guard<std::mutex> lock(log_mutex);
  std::ostream& os = (level == LogLevel::kError) ? std::cerr : std::cout;
  os << '[' << NowString() << "] [" << ToString(level) << "] " << message
     << '\n';
}

void Info(std::string_view message) {
  Log(LogLevel::kInfo, message);
}

void Warn(std::string_view message) {
  Log(LogLevel::kWarn, message);
}

void Error(std::string_view message) {
  Log(LogLevel::kError, message);
}

}  // namespace hp::base
