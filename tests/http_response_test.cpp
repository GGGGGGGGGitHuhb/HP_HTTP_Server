#include "http/http_response.h"

#include <cassert>
#include <string>

int main() {
  hp::http::HttpResponse ok(200, "OK");
  ok.SetHeader("Content-Type", "text/plain; charset=utf-8");
  ok.SetBody("hello");
  const std::string ok_raw = ok.ToString();
  assert(ok_raw.find("HTTP/1.1 200 OK\r\n") == 0);
  assert(ok_raw.find("Content-Length: 5\r\n") != std::string::npos);
  assert(ok_raw.find("Connection: close\r\n") != std::string::npos);
  assert(ok_raw.ends_with("\r\n\r\nhello"));

  const auto not_found =
      hp::http::HttpResponse::Text(404, "Not Found", "missing").ToString();
  assert(not_found.find("HTTP/1.1 404 Not Found\r\n") == 0);
  assert(not_found.find("missing") != std::string::npos);

  assert(hp::http::HttpResponse::MimeType("index.html") ==
         "text/html; charset=utf-8");
  assert(hp::http::HttpResponse::MimeType("style.css") ==
         "text/css; charset=utf-8");
  assert(hp::http::HttpResponse::MimeType("image.png") == "image/png");

  return 0;
}
