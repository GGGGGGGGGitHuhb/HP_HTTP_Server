#include "http/HttpServer.h"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <string>

int main() {
    const auto root = std::filesystem::temp_directory_path() / "hp_http_server_static_test";
    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root / "assets");
    {
        std::ofstream(root / "index.html") << "home";
        std::ofstream(root / "assets" / "app.txt") << "asset";
    }

    hp::http::HttpServer server(root);

    int status = 0;
    auto path = server.resolvePathForTest("/", status);
    assert(status == 200);
    assert(path.filename() == "index.html");

    path = server.resolvePathForTest("/assets/app.txt", status);
    assert(status == 200);
    assert(path.filename() == "app.txt");

    path = server.resolvePathForTest("/../../etc/passwd", status);
    assert(status == 403);

    path = server.resolvePathForTest("/%2e%2e/%2e%2e/etc/passwd", status);
    assert(status == 403);

    path = server.resolvePathForTest("/not-exist", status);
    assert(status == 404);

    const std::string ok = server.handleRequest("GET / HTTP/1.1\r\nHost: localhost\r\n\r\n");
    assert(ok.find("HTTP/1.1 200 OK\r\n") == 0);
    assert(ok.find("home") != std::string::npos);

    const std::string method = server.handleRequest("POST / HTTP/1.1\r\n\r\n");
    assert(method.find("HTTP/1.1 405 Method Not Allowed\r\n") == 0);

    const std::string bad = server.handleRequest("GET /\r\n\r\n");
    assert(bad.find("HTTP/1.1 400 Bad Request\r\n") == 0);

    std::filesystem::remove_all(root);
    return 0;
}
