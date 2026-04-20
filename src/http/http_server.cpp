#include "http/http_server.h"

#include <fstream>
#include <sstream>
#include <system_error>

#include "http/http_request.h"
#include "http/http_response.h"

namespace hp::http {
namespace {

std::string HtmlError(int status_code, const std::string& reason) {
  return "<!doctype html><html><head><meta charset=\"utf-8\"><title>" +
         std::to_string(status_code) + " " + reason +
         "</title></head><body><h1>" + std::to_string(status_code) + " " +
         reason + "</h1></body></html>";
}

std::string BuildErrorResponse(int status_code) {
  const std::string reason = HttpResponse::ReasonPhrase(status_code);
  HttpResponse response(status_code, reason);
  response.SetHeader("Content-Type", "text/html; charset=utf-8");
  response.SetBody(HtmlError(status_code, reason));
  if (status_code == 405) {
    response.SetHeader("Allow", "GET");
  }
  return response.ToString();
}

bool HasPathPrefix(const std::filesystem::path& path,
                   const std::filesystem::path& prefix) {
  auto path_it = path.begin();
  auto prefix_it = prefix.begin();
  for (; prefix_it != prefix.end(); ++prefix_it, ++path_it) {
    if (path_it == path.end() || *path_it != *prefix_it) {
      return false;
    }
  }
  return true;
}

}  // namespace

HttpServer::HttpServer(std::filesystem::path static_root) {
  std::error_code ec;
  static_root_ = std::filesystem::weakly_canonical(std::move(static_root), ec);
  if (ec) {
    static_root_ = std::filesystem::absolute(
        static_root_);  // static_root_ 保存静态文件根目录的绝对路径
  }
}

std::string HttpServer::HandleRequest(const std::string& raw_request) const {
  HttpRequest request;
  if (!request.Parse(raw_request)) {
    return BuildErrorResponse(400);
  }
  if (request.Method() != "GET") {
    return BuildErrorResponse(405);
  }

  int status_code = 200;
  const auto file_path = ResolvePath(request.Path(), status_code);
  if (status_code != 200) {
    return BuildErrorResponse(status_code);
  }
  return BuildFileResponse(file_path);
}

std::filesystem::path HttpServer::ResolvePathForTest(
    const std::string& request_path, int& status_code) const {
  return ResolvePath(request_path, status_code);
}

std::string HttpServer::BuildFileResponse(
    const std::filesystem::path& file_path) const {
  std::ifstream input(file_path, std::ios::binary);
  if (!input) {
    return BuildErrorResponse(500);
  }

  std::ostringstream body;
  body << input.rdbuf();
  if (input.bad()) {
    return BuildErrorResponse(500);
  }

  HttpResponse response(200, HttpResponse::ReasonPhrase(200));
  response.SetHeader("Content-Type",
                     HttpResponse::MimeType(file_path.string()));
  response.SetBody(body.str());
  return response.ToString();
}

std::filesystem::path HttpServer::ResolvePath(const std::string& request_path,
                                              int& status_code) const {
  status_code = 200;
  bool decode_ok = true;
  std::string decoded =
      UrlDecode(StripQueryAndFragment(request_path), decode_ok);
  if (!decode_ok || decoded.empty() || decoded[0] != '/') {
    status_code = 400;
    return {};
  }
  if (decoded.find('\0') != std::string::npos) {
    status_code = 400;
    return {};
  }
  if (decoded == "/") {
    decoded = "/index.html";
  }

  const std::filesystem::path relative = decoded.substr(1);
  std::error_code ec;
  const auto candidate =
      std::filesystem::weakly_canonical(static_root_ / relative, ec);
  if (ec) {
    const auto parent = std::filesystem::weakly_canonical(
        (static_root_ / relative).parent_path(), ec);
    if (ec || !HasPathPrefix(parent, static_root_)) {
      status_code = 403;
      return {};
    }
    status_code = 404;
    return {};
  }

  if (!HasPathPrefix(candidate, static_root_)) {
    status_code = 403;
    return {};
  }
  if (!std::filesystem::exists(candidate) ||
      !std::filesystem::is_regular_file(candidate)) {
    status_code = 404;
    return {};
  }
  return candidate;
}

std::string HttpServer::UrlDecode(const std::string& value, bool& ok) {
  ok = true;
  std::string decoded;
  decoded.reserve(value.size());
  for (std::size_t i = 0; i < value.size(); ++i) {
    if (value[i] == '%') {
      if (i + 2 >= value.size() || !IsHex(value[i + 1]) ||
          !IsHex(value[i + 2])) {
        ok = false;
        return {};
      }
      decoded.push_back(static_cast<char>((HexValue(value[i + 1]) << 4) |
                                          HexValue(value[i + 2])));
      i += 2;
    } else {
      decoded.push_back(value[i]);
    }
  }
  return decoded;
}

bool HttpServer::IsHex(char ch) {
  return ('0' <= ch && ch <= '9') || ('a' <= ch && ch <= 'f') ||
         ('A' <= ch && ch <= 'F');
}

int HttpServer::HexValue(char ch) {
  if ('0' <= ch && ch <= '9') {
    return ch - '0';
  }
  if ('a' <= ch && ch <= 'f') {
    return ch - 'a' + 10;
  }
  return ch - 'A' + 10;
}

std::string HttpServer::StripQueryAndFragment(const std::string& target) {
  const auto pos = target.find_first_of("?#");
  return pos == std::string::npos ? target : target.substr(0, pos);
}

}  // namespace hp::http
