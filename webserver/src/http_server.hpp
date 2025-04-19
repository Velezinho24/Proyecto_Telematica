// http_server.hpp
#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <sys/socket.h>
#include <netinet/in.h>

class HTTPServer
{
public:
    HTTPServer(int port, const std::string &log_file, const std::string &doc_root);
    ~HTTPServer();

    void start();
    void stop();

private:
    void run();
    void handle_client(int client_socket);
    void log_request(const std::string &request, int response_code);
    std::string get_mime_type(const std::string& path);

    int port_;
    std::string log_file_;
    std::string doc_root_;
    int server_fd_;
    bool running_;
    std::vector<std::thread> worker_threads_;
    std::mutex log_mutex_;
};

#endif // HTTP_SERVER_HPP