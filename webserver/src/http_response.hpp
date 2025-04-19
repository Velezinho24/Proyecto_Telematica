// http_response.hpp
#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include <string>
#include <map>

class HTTPResponse
{
public:
    HTTPResponse();

    void set_status(int code, const std::string &message);
    void set_header(const std::string &name, const std::string &value);
    void set_body(const std::string &body);

    int get_status_code() const { return status_code_; }
    std::string to_string() const;

private:
    int status_code_;
    std::string status_message_;
    std::map<std::string, std::string> headers_;
    std::string body_;
};

#endif // HTTP_RESPONSE_HPP