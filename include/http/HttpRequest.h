#pragma once

#include <string>
#include <unordered_map>

// 将原始请求字符串解析为结构化字段

namespace hp::http {

class HttpRequest {
public:
    bool parse(const std::string& raw_request);

    const std::string& method() const { return method_; }
    const std::string& path() const { return path_; }
    const std::string& version() const { return version_; }
    const std::unordered_map<std::string, std::string>& headers() const { return headers_; }
    const std::string& error() const { return error_; }

private:
    static std::string trim(std::string value);

    std::string method_;
    std::string path_;
    std::string version_;
    std::unordered_map<std::string, std::string> headers_;
    std::string error_;
};

} // namespace hp::http
