#include "base/Logger.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>

namespace hp::base {
namespace {

std::mutex g_log_mutex;

const char* toString(LogLevel level) {
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

std::string nowString() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
    localtime_r(&time, &tm);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

} // namespace

void log(LogLevel level, std::string_view message) {
    std::lock_guard<std::mutex> lock(g_log_mutex);
    std::ostream& os = (level == LogLevel::kError) ? std::cerr : std::cout;
    os << '[' << nowString() << "] [" << toString(level) << "] " << message << '\n';
}

void info(std::string_view message) {
    log(LogLevel::kInfo, message);
}

void warn(std::string_view message) {
    log(LogLevel::kWarn, message);
}

void error(std::string_view message) {
    log(LogLevel::kError, message);
}

} // namespace hp::base
