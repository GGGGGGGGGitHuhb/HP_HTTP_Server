#pragma once

#include <string>
#include <unordered_map>

namespace hp::http {

/**
 * @brief M1 HTTP 请求对象与最小请求头解析器。
 *
 * 只解析请求行和 header，不处理请求体；完整 HTTP 状态机留到后续里程碑。
 */
class HttpRequest {
 public:
  /**
   * @brief 解析原始 HTTP 请求头。
   *
   * @param raw_request 以 \r\n\r\n 结束的原始请求字符串。
   * @return 解析成功返回 true；失败时可通过 Error() 查看原因。
   */
  bool Parse(const std::string& raw_request);

  const std::string& Method() const {
    return method_;
  }
  const std::string& Path() const {
    return path_;
  }
  const std::string& Version() const {
    return version_;
  }
  const std::unordered_map<std::string, std::string>& Headers() const {
    return headers_;
  }
  const std::string& Error() const {
    return error_;
  }

 private:
  static std::string Trim(std::string value);

  std::string method_;
  std::string path_;
  std::string version_;
  std::unordered_map<std::string, std::string> headers_;
  std::string error_;
};

}  // namespace hp::http
