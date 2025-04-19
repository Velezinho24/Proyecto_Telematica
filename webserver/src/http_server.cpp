// http_server.cpp
#include "http_server.hpp"
#include "http_request.hpp"
#include "http_response.hpp"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <ctime>
#include <sys/stat.h>
#include <dirent.h>

HTTPServer::HTTPServer(int port, const std::string &log_file, const std::string &doc_root)
    : port_(port), log_file_(log_file), doc_root_(doc_root), server_fd_(-1), running_(false)
{
    // Asegurar que el directorio raíz termine con '/'
    if (doc_root_.back() != '/')
    {
        doc_root_ += '/';
    }
}

HTTPServer::~HTTPServer()
{
    stop();
}

void HTTPServer::start()
{
    // Crear socket
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0)
    {
        throw std::runtime_error("Error al crear el socket");
    }

    // Configurar opciones del socket
    int opt = 1;
    if (setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        throw std::runtime_error("Error al configurar opciones del socket");
    }

    // Configurar dirección
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port_);

    // Enlazar socket al puerto
    if (bind(server_fd_, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        throw std::runtime_error("Error al enlazar el socket");
    }

    // Escuchar conexiones
    if (listen(server_fd_, 10) < 0)
    {
        throw std::runtime_error("Error al escuchar conexiones");
    }

    running_ = true;
    std::thread main_thread(&HTTPServer::run, this);
    main_thread.detach();

    std::cout << "Servidor HTTP iniciado en el puerto " << port_ << std::endl;
}

void HTTPServer::stop()
{
    running_ = false;
    if (server_fd_ != -1)
    {
        close(server_fd_);
        server_fd_ = -1;
    }
}

void HTTPServer::run()
{
    while (running_)
    {
        struct sockaddr_in client_address;
        socklen_t client_addrlen = sizeof(client_address);

        int client_socket = accept(server_fd_, (struct sockaddr *)&client_address, &client_addrlen);
        if (client_socket < 0)
        {
            if (running_)
            {
                std::cerr << "Error al aceptar conexión" << std::endl;
            }
            continue;
        }

        // Manejar cliente en un nuevo hilo
        worker_threads_.emplace_back(&HTTPServer::handle_client, this, client_socket);
    }

    // Esperar a que todos los hilos terminen
    for (auto &thread : worker_threads_)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
    worker_threads_.clear();
}

void HTTPServer::handle_client(int client_socket)
{
    char buffer[4096] = {0};
    ssize_t bytes_read = read(client_socket, buffer, sizeof(buffer));

    if (bytes_read > 0)
    {
        HTTPRequest request(buffer);
        HTTPResponse response;

        // Procesar la solicitud
        if (request.get_method() == "GET")
        {
            std::string file_path = doc_root_ + request.get_uri();
            std::cout << "Buscando archivo en: " << file_path << std::endl;

            // Prevenir directory traversal
            if (file_path.find("../") != std::string::npos)
            {
                response.set_status(403, "Forbidden");
                response.set_body("<h1>403 Forbidden</h1>");
            }
            else
            {
                std::ifstream file(file_path, std::ios::binary);
                if (file)
                {
                    std::string content((std::istreambuf_iterator<char>(file)),
                                        std::istreambuf_iterator<char>());

                    response.set_status(200, "OK");
                    response.set_header("Content-Type", get_mime_type(request.get_uri()));
                    response.set_body(content);
                }
                else
                {
                    response.set_status(404, "Not Found");
                    response.set_body("<h1>404 Not Found</h1>");
                }
            }
        }
        else if (request.get_method() == "HEAD")
        {
            // Similar a GET pero sin cuerpo
            response.set_status(200, "OK");
            response.set_header("Content-Type", "text/html");
        }
        else if (request.get_method() == "POST")
        {
            // Procesar datos POST
            response.set_status(200, "OK");
            response.set_body("<h1>POST recibido</h1><p>" + request.get_body() + "</p>");
        }
        else
        {
            response.set_status(400, "Bad Request");
            response.set_body("<h1>400 Bad Request</h1>");
        }

        // Enviar respuesta
        std::string response_str = response.to_string();
        send(client_socket, response_str.c_str(), response_str.size(), 0);

        // Registrar la solicitud
        log_request(request.get_method() + " " + request.get_uri(), response.get_status_code());
    }

    close(client_socket);
}

void HTTPServer::log_request(const std::string &request, int response_code)
{
    std::lock_guard<std::mutex> lock(log_mutex_);

    std::ofstream log_file(log_file_, std::ios::app);
    if (log_file)
    {
        std::time_t now = std::time(nullptr);
        char time_str[100];
        std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

        log_file << "[" << time_str << "] " << request << " - " << response_code << std::endl;
    }
}

std::string HTTPServer::get_mime_type(const std::string& path) {
    std::string extension = path.substr(path.find_last_of('.') + 1);
    
    if (extension == "html" || extension == "htm") return "text/html";
    if (extension == "css") return "text/css";
    if (extension == "js") return "application/javascript";
    if (extension == "jpg" || extension == "jpeg") return "image/jpeg";
    if (extension == "png") return "image/png";
    if (extension == "gif") return "image/gif";
    if (extension == "pdf") return "application/pdf";
    if (extension == "json") return "application/json";
    
    return "text/plain";
}