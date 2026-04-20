#pragma once

#include <filesystem>
#include <string>

namespace hp::http {

/**
 * @brief M1 HTTP 静态文件服务。
 *
 * HttpServer 不直接操作 socket/epoll，只负责把原始 HTTP 请求转换为原始 HTTP
 * 响应。 所有静态文件访问都会限制在 static_root 内，防止路径穿越。
 */
class HttpServer {
 public:
  /**
   * @brief 创建静态文件服务器。
   *
   * @param static_root 静态文件根目录。
   */
  explicit HttpServer(std::filesystem::path static_root);

  /**
   * @brief 处理原始 HTTP 请求并返回原始 HTTP 响应。
   */
  std::string HandleRequest(const std::string& raw_request) const;

  /**
   * @brief 暴露路径解析能力给单元测试。
   *
   * @param request_path HTTP request-target 中的路径部分。
   * @param status_code 输出解析结果对应的 HTTP 状态码。
   * @return 解析成功时返回静态文件路径；失败时返回空路径。
   */
  std::filesystem::path ResolvePathForTest(const std::string& request_path,
                                           int& status_code) const;

 private:
  std::string BuildFileResponse(const std::filesystem::path& file_path) const;
  std::filesystem::path ResolvePath(const std::string& request_path,
                                    int& status_code) const;
  static std::string UrlDecode(const std::string& value, bool& ok);
  static bool IsHex(char ch);
  static int HexValue(char ch);
  static std::string StripQueryAndFragment(const std::string& target);

  std::filesystem::path static_root_;
};

}  // namespace hp::http
