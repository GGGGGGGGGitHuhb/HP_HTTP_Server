#include "http/HttpRequest.h"

#include <cassert>

int main() {
    hp::http::HttpRequest root;
    assert(root.parse("GET / HTTP/1.1\r\nHost: localhost\r\n\r\n"));
    assert(root.method() == "GET");
    assert(root.path() == "/");
    assert(root.version() == "HTTP/1.1");
    assert(root.headers().at("Host") == "localhost");

    hp::http::HttpRequest index;
    assert(index.parse("GET /index.html HTTP/1.1\r\n\r\n"));
    assert(index.path() == "/index.html");

    hp::http::HttpRequest bad_line;
    assert(!bad_line.parse("GET /\r\n\r\n"));

    hp::http::HttpRequest post;
    assert(post.parse("POST / HTTP/1.1\r\n\r\n"));
    assert(post.method() == "POST");

    hp::http::HttpRequest bad_header;
    assert(!bad_header.parse("GET / HTTP/1.1\r\nBadHeader\r\n\r\n"));

    return 0;
}
