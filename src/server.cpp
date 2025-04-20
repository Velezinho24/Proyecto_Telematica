// server.cpp
// HTTP/1.1 Web Server - Proyecto Telemática
// Uso: ./server <HTTP_PORT> <LOG_FILE> <DOCUMENT_ROOT>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#include <csignal>
#include <cstdarg>
#include <cstring>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <ctime>

constexpr int BACKLOG = 10;
constexpr size_t BUF_SIZE = 8192;

std::ofstream logFile;
std::mutex logMutex;
std::string documentRoot;
int serverFd;

void logPrintf(const char* fmt, ...) {
    std::lock_guard<std::mutex> lock(logMutex);
    auto now = std::time(nullptr);
    std::tm tm = *std::localtime(&now);
    char timeBuf[20];
    std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", &tm);
    logFile << timeBuf << " ";
    
    char msgBuf[1024];
    va_list args;
    va_start(args, fmt);
    std::vsnprintf(msgBuf, sizeof(msgBuf), fmt, args);
    va_end(args);

    logFile << msgBuf << std::endl;
}

std::string getMimeType(const std::string& path) {
    auto pos = path.rfind('.');
    if (pos == std::string::npos) return "application/octet-stream";
    std::string ext = path.substr(pos);
    if (ext == ".html" || ext == ".htm") return "text/html";
    if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
    if (ext == ".png") return "image/png";
    if (ext == ".css") return "text/css";
    if (ext == ".js")  return "application/javascript";
    if (ext == ".json") return "application/json";
    if (ext == ".txt") return "text/plain";
    if (ext == ".mp4") return "video/mp4";
    return "application/octet-stream";
}

void sendResponse(int clientFd, int statusCode, const std::string& statusMsg,
                  const std::string& contentType, const char* body, size_t bodyLen) {
    std::ostringstream header;
    header << "HTTP/1.1 " << statusCode << " " << statusMsg << "\r\n"
           << "Server: TelematicaServer/1.0\r\n"
           << "Content-Length: " << bodyLen << "\r\n"
           << "Content-Type: " << contentType << "\r\n"
           << "Connection: close\r\n"
           << "\r\n";
    std::string hdr = header.str();
    send(clientFd, hdr.c_str(), hdr.size(), 0);
    if (body && bodyLen > 0) {
        send(clientFd, body, bodyLen, 0);
    }
    logPrintf("Responded %d %s", statusCode, statusMsg.c_str());
}

void notFound(int clientFd) {
    static const char body[] = "<html><body><h1>404 Not Found</h1></body></html>";
    sendResponse(clientFd, 404, "Not Found", "text/html", body, sizeof(body)-1);
}

void badRequest(int clientFd) {
    static const char body[] = "<html><body><h1>400 Bad Request</h1></body></html>";
    sendResponse(clientFd, 400, "Bad Request", "text/html", body, sizeof(body)-1);
}

void serveFile(int clientFd, const std::string& path, bool headOnly) {
    std::string fullPath = documentRoot + path;
    struct stat st;
    if (stat(fullPath.c_str(), &st) < 0 || S_ISDIR(st.st_mode)) {
        notFound(clientFd);
        return;
    }
    std::ifstream file(fullPath, std::ios::binary);
    if (!file) {
        notFound(clientFd);
        return;
    }
    size_t filesize = st.st_size;
    std::string mime = getMimeType(fullPath);
    if (headOnly) {
        sendResponse(clientFd, 200, "OK", mime, nullptr, filesize);
    } else {
        std::vector<char> buffer(filesize);
        file.read(buffer.data(), filesize);
        sendResponse(clientFd, 200, "OK", mime, buffer.data(), filesize);
    }
}

void handleClient(int clientFd) {
    char buf[BUF_SIZE];
    ssize_t len = recv(clientFd, buf, sizeof(buf)-1, 0);
    if (len <= 0) {
        close(clientFd);
        return;
    }
    buf[len] = '\0';
    logPrintf("Request:\n%s", buf);

    std::istringstream request(buf);
    std::string method, path;
    if (!(request >> method >> path)) {
        badRequest(clientFd);
        close(clientFd);
        return;
    }
    bool headOnly = (method == "HEAD");
    if (method == "GET" || method == "HEAD") {
        if (path == "/") path = "/index.html";
        serveFile(clientFd, path, headOnly);
    } else if (method == "POST") {
        static const char body[] = "<html><body><h1>POST recibido</h1></body></html>";
        sendResponse(clientFd, 200, "OK", "text/html", body, sizeof(body)-1);
    } else {
        badRequest(clientFd);
    }
    close(clientFd);
}

void sigintHandler(int) {
    logPrintf("Shutting down server");
    logFile.close();
    close(serverFd);
    std::exit(0);
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <HTTP_PORT> <LOG_FILE> <DOCUMENT_ROOT>\n";
        return EXIT_FAILURE;
    }
    int port = std::stoi(argv[1]);
    logFile.open(argv[2], std::ios::app);
    if (!logFile) {
        std::perror("Opening log file");
        return EXIT_FAILURE;
    }
    documentRoot = argv[3];

    std::signal(SIGINT, sigintHandler);

    struct addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    struct addrinfo* res;
    if (getaddrinfo(nullptr, argv[1], &hints, &res) != 0) {
        std::perror("getaddrinfo");
        return EXIT_FAILURE;
    }
    serverFd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    int yes = 1;
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    if (bind(serverFd, res->ai_addr, res->ai_addrlen) < 0) {
        std::perror("bind");
        return EXIT_FAILURE;
    }
    freeaddrinfo(res);
    if (listen(serverFd, BACKLOG) < 0) {
        std::perror("listen");
        return EXIT_FAILURE;
    }
    logPrintf("Server started on port %d, serving %s", port, documentRoot.c_str());

    while (true) {
        struct sockaddr_storage clientAddr;
        socklen_t addrLen = sizeof(clientAddr);
        int clientFd = accept(serverFd, (struct sockaddr*)&clientAddr, &addrLen);
        if (clientFd < 0) {
            std::perror("accept");
            continue;
        }
        std::thread(handleClient, clientFd).detach();
    }
    return 0;
}
