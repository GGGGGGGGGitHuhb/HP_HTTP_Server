#include "http/HttpResponse.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <sstream>

namespace hp::http {

HttpResponse::HttpResponse(int status_code, std::string reason_phrase)
    : status_code_(status_code), reason_phrase_(std::move(reason_phrase)) {}

void HttpResponse::setHeader(std::string name, std::string value) {
    headers_.emplace_back(std::move(name), std::move(value));
}

void HttpResponse::setBody(std::string body) {
    body_ = std::move(body);
}

std::string HttpResponse::toString() const {
    std::ostringstream oss;
    oss << "HTTP/1.1 " << status_code_ << ' ' << reason_phrase_ << "\r\n";
    for (const auto& [name, value] : headers_) {
        oss << name << ": " << value << "\r\n";
    }
    oss << "Content-Length: " << body_.size() << "\r\n";
    oss << "Connection: close\r\n";
    oss << "\r\n";
    oss << body_;
    return oss.str();
}

HttpResponse HttpResponse::text(int status_code, const std::string& reason_phrase, const std::string& body) {
    HttpResponse response(status_code, reason_phrase);
    response.setHeader("Content-Type", "text/plain; charset=utf-8");
    response.setBody(body);
    return response;
}

std::string HttpResponse::reasonPhrase(int status_code) {
    switch (status_code) {
    case 200:
        return "OK";
    case 400:
        return "Bad Request";
    case 403:
        return "Forbidden";
    case 404:
        return "Not Found";
    case 405:
        return "Method Not Allowed";
    case 500:
        return "Internal Server Error";
    default:
        return "Unknown";
    }
}

std::string HttpResponse::mimeType(const std::string& path) {
    auto extension = std::filesystem::path(path).extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });

    if (extension == ".html" || extension == ".htm") {
        return "text/html; charset=utf-8";
    }
    if (extension == ".css") {
        return "text/css; charset=utf-8";
    }
    if (extension == ".js") {
        return "application/javascript; charset=utf-8";
    }
    if (extension == ".json") {
        return "application/json; charset=utf-8";
    }
    if (extension == ".txt") {
        return "text/plain; charset=utf-8";
    }
    if (extension == ".png") {
        return "image/png";
    }
    if (extension == ".jpg" || extension == ".jpeg") {
        return "image/jpeg";
    }
    return "application/octet-stream";
}

} // namespace hp::http
