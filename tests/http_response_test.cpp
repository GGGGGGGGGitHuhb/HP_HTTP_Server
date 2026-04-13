#include "http/HttpResponse.h"

#include <cassert>
#include <string>

int main() {
    hp::http::HttpResponse ok(200, "OK");
    ok.setHeader("Content-Type", "text/plain; charset=utf-8");
    ok.setBody("hello");
    const std::string ok_raw = ok.toString();
    assert(ok_raw.find("HTTP/1.1 200 OK\r\n") == 0);
    assert(ok_raw.find("Content-Length: 5\r\n") != std::string::npos);
    assert(ok_raw.find("Connection: close\r\n") != std::string::npos);
    assert(ok_raw.ends_with("\r\n\r\nhello"));

    const auto not_found = hp::http::HttpResponse::text(404, "Not Found", "missing").toString();
    assert(not_found.find("HTTP/1.1 404 Not Found\r\n") == 0);
    assert(not_found.find("missing") != std::string::npos);

    assert(hp::http::HttpResponse::mimeType("index.html") == "text/html; charset=utf-8");
    assert(hp::http::HttpResponse::mimeType("style.css") == "text/css; charset=utf-8");
    assert(hp::http::HttpResponse::mimeType("image.png") == "image/png");

    return 0;
}
