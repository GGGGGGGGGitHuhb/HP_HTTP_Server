#pragma once

#include <string>
#include <vector>

namespace hp::http {

/**
 * @brief HTTP 响应构造器。
 *
 * 负责生成状态行、响应头和响应体；M1 默认在 ToString() 中加入 Connection:
 * close。
 */
class HttpResponse {
 public:
  /**
   * @brief 创建指定状态码和原因短语的响应。
   */
  HttpResponse(int status_code, std::string reason_phrase);

  /**
   * @brief 添加一个响应头。
   */
  void SetHeader(std::string name, std::string value);

  /**
   * @brief 设置响应体。
   */
  void SetBody(std::string body);

  /**
   * @brief 序列化为可直接写入 socket 的 HTTP 响应字符串。
   */
  std::string ToString() const;

  /**
   * @brief 构造 text/plain 响应。
   */
  static HttpResponse Text(int status_code, const std::string& reason_phrase,
                           const std::string& body);

  /**
   * @brief 返回常用 HTTP 状态码的原因短语。
   */
  static std::string ReasonPhrase(int status_code);

  /**
   * @brief 根据文件扩展名返回基础 MIME 类型。
   */
  static std::string MimeType(const std::string& path);

 private:
  int status_code_;
  std::string reason_phrase_;
  std::vector<std::pair<std::string, std::string>> headers_;
  std::string body_;
};

}  // namespace hp::http
