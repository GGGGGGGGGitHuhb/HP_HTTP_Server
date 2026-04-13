#pragma once

#include <string>
#include <vector>

namespace hp::http {

class HttpResponse {
public:
    HttpResponse(int status_code, std::string reason_phrase);

    void setHeader(std::string name, std::string value);
    void setBody(std::string body);
    std::string toString() const;

    static HttpResponse text(int status_code, const std::string& reason_phrase, const std::string& body);
    static std::string reasonPhrase(int status_code);
    static std::string mimeType(const std::string& path);

private:
    int status_code_;
    std::string reason_phrase_;
    std::vector<std::pair<std::string, std::string>> headers_;
    std::string body_;
};

} // namespace hp::http
