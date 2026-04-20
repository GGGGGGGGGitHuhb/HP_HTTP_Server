#include "http/http_request.h"

#include <cassert>

int main() {
  hp::http::HttpRequest root;
  assert(root.Parse("GET / HTTP/1.1\r\nHost: localhost\r\n\r\n"));
  assert(root.Method() == "GET");
  assert(root.Path() == "/");
  assert(root.Version() == "HTTP/1.1");
  assert(root.Headers().at("Host") == "localhost");

  hp::http::HttpRequest index;
  assert(index.Parse("GET /index.html HTTP/1.1\r\n\r\n"));
  assert(index.Path() == "/index.html");

  hp::http::HttpRequest bad_line;
  assert(!bad_line.Parse("GET /\r\n\r\n"));

  hp::http::HttpRequest post;
  assert(post.Parse("POST / HTTP/1.1\r\n\r\n"));
  assert(post.Method() == "POST");

  hp::http::HttpRequest bad_header;
  assert(!bad_header.Parse("GET / HTTP/1.1\r\nBadHeader\r\n\r\n"));

  return 0;
}
