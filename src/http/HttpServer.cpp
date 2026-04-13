#include "http/HttpServer.h"

#include "http/HttpRequest.h"
#include "http/HttpResponse.h"

#include <fstream>
#include <sstream>
#include <system_error>

namespace hp::http {
namespace {

std::string htmlError(int status_code, const std::string& reason) {
    return "<!doctype html><html><head><meta charset=\"utf-8\"><title>" + std::to_string(status_code) + " " + reason +
           "</title></head><body><h1>" + std::to_string(status_code) + " " + reason + "</h1></body></html>";
}

std::string buildErrorResponse(int status_code) {
    const std::string reason = HttpResponse::reasonPhrase(status_code);
    HttpResponse response(status_code, reason);
    response.setHeader("Content-Type", "text/html; charset=utf-8");
    response.setBody(htmlError(status_code, reason));
    if (status_code == 405) {
        response.setHeader("Allow", "GET");
    }
    return response.toString();
}

bool hasPathPrefix(const std::filesystem::path& path, const std::filesystem::path& prefix) {
    auto path_it = path.begin();
    auto prefix_it = prefix.begin();
    for (; prefix_it != prefix.end(); ++prefix_it, ++path_it) {
        if (path_it == path.end() || *path_it != *prefix_it) {
            return false;
        }
    }
    return true;
}

} // namespace

HttpServer::HttpServer(std::filesystem::path static_root) {
    std::error_code ec;
    static_root_ = std::filesystem::weakly_canonical(std::move(static_root), ec);
    if (ec) {
        static_root_ = std::filesystem::absolute(static_root_);
    }
}

std::string HttpServer::handleRequest(const std::string& raw_request) const {
    HttpRequest request;
    if (!request.parse(raw_request)) {
        return buildErrorResponse(400);
    }
    if (request.method() != "GET") {
        return buildErrorResponse(405);
    }

    int status_code = 200;
    const auto file_path = resolvePath(request.path(), status_code);
    if (status_code != 200) {
        return buildErrorResponse(status_code);
    }
    return buildFileResponse(file_path);
}

std::filesystem::path HttpServer::resolvePathForTest(const std::string& request_path, int& status_code) const {
    return resolvePath(request_path, status_code);
}

std::string HttpServer::buildFileResponse(const std::filesystem::path& file_path) const {
    std::ifstream input(file_path, std::ios::binary);
    if (!input) {
        return buildErrorResponse(500);
    }

    std::ostringstream body;
    body << input.rdbuf();
    if (input.bad()) {
        return buildErrorResponse(500);
    }

    HttpResponse response(200, HttpResponse::reasonPhrase(200));
    response.setHeader("Content-Type", HttpResponse::mimeType(file_path.string()));
    response.setBody(body.str());
    return response.toString();
}

std::filesystem::path HttpServer::resolvePath(const std::string& request_path, int& status_code) const {
    status_code = 200;
    bool decode_ok = true;
    std::string decoded = urlDecode(stripQueryAndFragment(request_path), decode_ok);
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
    const auto candidate = std::filesystem::weakly_canonical(static_root_ / relative, ec);
    if (ec) {
        const auto parent = std::filesystem::weakly_canonical((static_root_ / relative).parent_path(), ec);
        if (ec || !hasPathPrefix(parent, static_root_)) {
            status_code = 403;
            return {};
        }
        status_code = 404;
        return {};
    }

    if (!hasPathPrefix(candidate, static_root_)) {
        status_code = 403;
        return {};
    }
    if (!std::filesystem::exists(candidate) || !std::filesystem::is_regular_file(candidate)) {
        status_code = 404;
        return {};
    }
    return candidate;
}

std::string HttpServer::urlDecode(const std::string& value, bool& ok) {
    ok = true;
    std::string decoded;
    decoded.reserve(value.size());
    for (std::size_t i = 0; i < value.size(); ++i) {
        if (value[i] == '%') {
            if (i + 2 >= value.size() || !isHex(value[i + 1]) || !isHex(value[i + 2])) {
                ok = false;
                return {};
            }
            decoded.push_back(static_cast<char>((hexValue(value[i + 1]) << 4) | hexValue(value[i + 2])));
            i += 2;
        } else {
            decoded.push_back(value[i]);
        }
    }
    return decoded;
}

bool HttpServer::isHex(char ch) {
    return ('0' <= ch && ch <= '9') || ('a' <= ch && ch <= 'f') || ('A' <= ch && ch <= 'F');
}

int HttpServer::hexValue(char ch) {
    if ('0' <= ch && ch <= '9') {
        return ch - '0';
    }
    if ('a' <= ch && ch <= 'f') {
        return ch - 'a' + 10;
    }
    return ch - 'A' + 10;
}

std::string HttpServer::stripQueryAndFragment(const std::string& target) {
    const auto pos = target.find_first_of("?#");
    return pos == std::string::npos ? target : target.substr(0, pos);
}

} // namespace hp::http
