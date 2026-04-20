#include "http/http_request.h"

#include <cctype>
#include <sstream>

namespace hp::http {

bool HttpRequest::Parse(const std::string& raw_request) {
  method_.clear();
  path_.clear();
  version_.clear();
  headers_.clear();
  error_.clear();

  const auto header_end = raw_request.find("\r\n\r\n");
  if (header_end == std::string::npos) {
    error_ = "incomplete request headers";
    return false;
  }

  std::istringstream stream(raw_request.substr(0, header_end));
  std::string request_line;
  if (!std::getline(stream, request_line)) {
    error_ = "missing request line";
    return false;
  }
  if (!request_line.empty() && request_line.back() == '\r') {
    request_line.pop_back();
  }

  std::istringstream line_stream(request_line);
  std::string extra;
  if (!(line_stream >> method_ >> path_ >> version_) ||
      (line_stream >> extra)) {
    error_ = "invalid request line";
    return false;
  }

  if (path_.empty() || path_[0] != '/') {
    error_ = "request target must start with /";
    return false;
  }
  if (version_ != "HTTP/1.0" && version_ != "HTTP/1.1") {
    error_ = "unsupported HTTP version format";
    return false;
  }

  std::string header_line;
  while (std::getline(stream, header_line)) {
    if (!header_line.empty() && header_line.back() == '\r') {
      header_line.pop_back();
    }
    if (header_line.empty()) {
      continue;
    }
    const auto colon = header_line.find(':');
    if (colon == std::string::npos || colon == 0) {
      error_ = "invalid header line";
      return false;
    }
    headers_[Trim(header_line.substr(0, colon))] =
        Trim(header_line.substr(colon + 1));
  }

  return true;
}

std::string HttpRequest::Trim(std::string value) {
  auto first = value.begin();
  while (first != value.end() &&
         std::isspace(static_cast<unsigned char>(*first))) {
    ++first;
  }
  auto last = value.end();
  while (last != first &&
         std::isspace(static_cast<unsigned char>(*(last - 1)))) {
    --last;
  }
  return {first, last};
}

}  // namespace hp::http
