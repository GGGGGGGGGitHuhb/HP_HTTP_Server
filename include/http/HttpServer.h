#pragma once

#include <filesystem>
#include <string>

// HttpServer 只关心一个 Http 请求该得到什么 Http 响应

namespace hp::http {

class HttpServer {
public:
    // 单参构造函数使用 explicit 防止参数隐式类型转换
    explicit HttpServer(std::filesystem::path static_root);

    std::string handleRequest(const std::string& raw_request) const; // 输入原始 HTTP 请求字符串，输出原始 HTTP 响应字符串
    std::filesystem::path resolvePathForTest(const std::string& request_path, int& status_code) const;

private:
    std::string buildFileResponse(const std::filesystem::path& file_path) const;
    std::filesystem::path resolvePath(const std::string& request_path, int& status_code) const;
    static std::string urlDecode(const std::string& value, bool& ok);
    static bool isHex(char ch);
    static int hexValue(char ch);
    static std::string stripQueryAndFragment(const std::string& target);

    std::filesystem::path static_root_; // 静态文件根目录
};

} // namespace hp::http
