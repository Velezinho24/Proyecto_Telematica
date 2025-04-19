// http_request.hpp
#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include <string>
#include <map>
#include <vector>

class HTTPRequest
{
public:
    HTTPRequest(const std::string &raw_request);

    const std::string &get_method() const { return method_; }
    const std::string &get_uri() const { return uri_; }
    const std::string &get_version() const { return version_; }
    const std::string &get_body() const { return body_; }
    const std::map<std::string, std::string> &get_headers() const { return headers_; }

private:
    void parse(const std::string &raw_request);

    std::string method_;
    std::string uri_;
    std::string version_;
    std::map<std::string, std::string> headers_;
    std::string body_;
};

#endif // HTTP_REQUEST_HPP