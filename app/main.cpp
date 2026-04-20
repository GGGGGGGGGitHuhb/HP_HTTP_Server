#include <charconv>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>
#include <string_view>

#include "base/logger.h"
#include "net/tcp_server.h"

// main.cpp 只负责服务器启动

namespace {

struct Options {
  uint16_t port_{8080};
  std::string root_{"public"};
};

void PrintUsage(const char* program) {
  std::cerr << "Usage: " << program << " [--port PORT] [--root DIR]\n";
}

bool ParsePort(std::string_view value, uint16_t& port) {
  unsigned int parsed = 0;
  const auto* begin = value.data();
  const auto* end =
      value.data() + value.size();  // end 指向最后一个字符的下一个位置
  const auto result = std::from_chars(begin, end, parsed);
  if (result.ec != std::errc{} || result.ptr != end || parsed == 0 ||
      parsed > 65535) {
    return false;
  }
  port = static_cast<uint16_t>(parsed);
  return true;
}

bool ParseOptions(int argc, char* argv[], Options& options) {
  for (int i = 1; i < argc; ++i) {
    const std::string_view arg(argv[i]);
    if (arg == "--port") {
      if (++i >= argc || !ParsePort(argv[i], options.port_)) {
        return false;
      }
    } else if (arg == "--root") {
      if (++i >= argc) {
        return false;
      }
      options.root_ = argv[i];
    } else if (arg == "--help" || arg == "-h") {
      PrintUsage(argv[0]);
      std::exit(0);
    } else {
      return false;
    }
  }
  return true;
}

}  // namespace

int main(int argc, char* argv[]) {
  // 解析命令行参数
  Options options;
  if (!ParseOptions(argc, argv, options)) {
    PrintUsage(argv[0]);
    return 2;
  }

  try {
    // 创建一个 TcpServer 并调用 Start()，服务器开始工作
    hp::net::TcpServer server(options.port_,
                              options.root_);  // 传端口和静态文件目录
    server.Start();
  } catch (const std::exception& ex) {
    // 若出错则打印日志并退出
    hp::base::Error(ex.what());
    return 1;
  }
  return 0;
}
