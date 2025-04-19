// http_request.cpp
#include "http_request.hpp"
#include <sstream>
#include <algorithm>

HTTPRequest::HTTPRequest(const std::string &raw_request)
{
    parse(raw_request);
}

void HTTPRequest::parse(const std::string &raw_request)
{
    std::istringstream request_stream(raw_request);
    std::string line;

    // Parsear primera línea (método, URI, versión)
    if (std::getline(request_stream, line))
    {
        std::istringstream line_stream(line);
        line_stream >> method_ >> uri_ >> version_;
    }

    // Parsear headers
    while (std::getline(request_stream, line) && line != "\r")
    {
        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos)
        {
            std::string key = line.substr(0, colon_pos);
            std::string value = line.substr(colon_pos + 1);

            // Eliminar espacios en blanco
            key.erase(std::remove_if(key.begin(), key.end(), ::isspace), key.end());
            value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());

            headers_[key] = value;
        }
    }

    // Parsear cuerpo (si existe)
    if (method_ == "POST" || method_ == "PUT")
    {
        std::ostringstream body_stream;
        while (std::getline(request_stream, line))
        {
            body_stream << line;
        }
        body_ = body_stream.str();
    }
}