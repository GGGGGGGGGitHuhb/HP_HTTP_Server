#pragma once

#include <string_view>

namespace hp::base {

enum class LogLevel {
    kInfo,
    kWarn,
    kError,
};

void log(LogLevel level, std::string_view message);
void info(std::string_view message);
void warn(std::string_view message);
void error(std::string_view message);

} // namespace hp::base
