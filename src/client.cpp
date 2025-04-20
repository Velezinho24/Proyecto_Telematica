// client.cpp
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

int main() {
    const char* host = "127.0.0.1";
    const char* port = "8080";
    const char* request =
        "GET /index.html HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Connection: close\r\n"
        "\r\n";

    // 1) Resolver dirección
    struct addrinfo hints{}, *res;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(host, port, &hints, &res) != 0) {
        perror("getaddrinfo");
        return 1;
    }

    // 2) Crear socket y conectar
    int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (connect(sock, res->ai_addr, res->ai_addrlen) < 0) {
        perror("connect");
        return 1;
    }
    freeaddrinfo(res);

    // 3) Enviar petición HTTP
    send(sock, request, strlen(request), 0);

    // 4) Leer respuesta y volcarla por pantalla
    char buf[1024];
    ssize_t n;
    while ((n = recv(sock, buf, sizeof(buf)-1, 0)) > 0) {
        buf[n] = '\0';
        std::cout << buf;
    }

    close(sock);
    return 0;
}
