#include "base/Logger.h"
#include "net/TcpServer.h"

#include <charconv>
#include <cstdlib>
#include <cstdint>
#include <exception>
#include <iostream>
#include <string>
#include <string_view>

// main.cpp 只负责服务器启动

namespace {

struct Options {
    uint16_t port{8080};
    std::string root{"public"};
};

void printUsage(const char* program) {
    std::cerr << "Usage: " << program << " [--port PORT] [--root DIR]\n";
}

bool parsePort(std::string_view value, uint16_t& port) {
    unsigned int parsed = 0;
    const auto* begin = value.data();
    const auto* end = value.data() + value.size(); // end 指向最后一个字符的下一个位置
    const auto result = std::from_chars(begin, end, parsed);
    if (result.ec != std::errc{} || result.ptr != end || parsed == 0 || parsed > 65535) {
        return false;
    }
    port = static_cast<uint16_t>(parsed);
    return true;
}

bool parseOptions(int argc, char* argv[], Options& options) {
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg(argv[i]);
        if (arg == "--port") {
            if (++i >= argc || !parsePort(argv[i], options.port)) {
                return false;
            }
        } else if (arg == "--root") {
            if (++i >= argc) {
                return false;
            }
            options.root = argv[i];
        } else if (arg == "--help" || arg == "-h") {
            printUsage(argv[0]);
            std::exit(0);
        } else {
            return false;
        }
    }
    return true;
}

} // namespace

int main(int argc, char* argv[]) {
    // 解析命令行参数
    Options options;
    if (!parseOptions(argc, argv, options)) {
        printUsage(argv[0]);
        return 2;
    }

    try {
        // 创建一个 TcpServer 并调用 start()，服务器开始工作
        hp::net::TcpServer server(options.port, options.root); // 传端口和静态文件目录
        server.start();
    } catch (const std::exception& ex) {
        // 若出错则打印日志并退出
        hp::base::error(ex.what());
        return 1;
    }
    return 0;
}
