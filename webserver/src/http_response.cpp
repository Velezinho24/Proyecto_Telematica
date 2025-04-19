// http_response.cpp
#include "http_response.hpp"
#include <sstream>

HTTPResponse::HTTPResponse() : status_code_(200), status_message_("OK") {}

void HTTPResponse::set_status(int code, const std::string &message)
{
    status_code_ = code;
    status_message_ = message;
}

void HTTPResponse::set_header(const std::string &name, const std::string &value)
{
    headers_[name] = value;
}

void HTTPResponse::set_body(const std::string &body)
{
    body_ = body;
    headers_["Content-Length"] = std::to_string(body.size());
}

std::string HTTPResponse::to_string() const
{
    std::ostringstream response_stream;

    // Línea de estado
    response_stream << "HTTP/1.1 " << status_code_ << " " << status_message_ << "\r\n";

    // Headers
    for (const auto &header : headers_)
    {
        response_stream << header.first << ": " << header.second << "\r\n";
    }

    // Separador y cuerpo
    response_stream << "\r\n"
                    << body_;

    return response_stream.str();
}