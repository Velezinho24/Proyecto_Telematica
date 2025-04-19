// server.cpp
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <vector>
#include <string>
#include <cstring>
#include <ctime>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

string logFile;
string rootFolder;

void logRequest(const string &message)
{
    ofstream log(logFile, ios::app);
    log << "[" << time(nullptr) << "] " << message << endl;
}

string getMimeType(const string &path)
{
    if (path.ends_with(".html"))
        return "text/html";
    if (path.ends_with(".jpg") || path.ends_with(".jpeg"))
        return "image/jpeg";
    if (path.ends_with(".png"))
        return "image/png";
    if (path.ends_with(".mp4"))
        return "video/mp4";
    return "text/plain";
}

void handleClient(int clientSock)
{
    char buffer[4096] = {0};
    read(clientSock, buffer, sizeof(buffer));
    string request(buffer);

    string method, path, version;
    istringstream reqStream(request);
    reqStream >> method >> path >> version;

    if (path == "/")
        path = "/index.html";

    string fullPath = rootFolder + path;
    if (method != "GET" && method != "HEAD" && method != "POST")
    {
        string response = "HTTP/1.1 400 Bad Request\r\n\r\n";
        send(clientSock, response.c_str(), response.size(), 0);
        logRequest("400 Bad Request: " + method + " " + path);
        close(clientSock);
        return;
    }

    ifstream file(fullPath, ios::binary);
    if (!file.is_open())
    {
        string response = "HTTP/1.1 404 Not Found\r\n\r\nPage Not Found";
        send(clientSock, response.c_str(), response.size(), 0);
        logRequest("404 Not Found: " + method + " " + path);
        close(clientSock);
        return;
    }

    file.seekg(0, ios::end);
    size_t filesize = file.tellg();
    file.seekg(0, ios::beg);
    vector<char> fileData(filesize);
    file.read(fileData.data(), filesize);

    string response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Length: " + to_string(filesize) + "\r\n";
    response += "Content-Type: " + getMimeType(path) + "\r\n";
    response += "\r\n";

    send(clientSock, response.c_str(), response.size(), 0);
    if (method != "HEAD")
        send(clientSock, fileData.data(), filesize, 0);

    logRequest("200 OK: " + method + " " + path);
    close(clientSock);
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        cerr << "Uso: ./server <HTTP_PORT> <LogFile> <DocumentRootFolder>\n";
        return 1;
    }

    int port = stoi(argv[1]);
    logFile = argv[2];
    rootFolder = argv[3];

    int serverSock = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSock == -1)
    {
        cerr << "Error creando el socket\n";
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSock, (sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        cerr << "Error en bind()\n";
        return 1;
    }

    listen(serverSock, 10);
    cout << "Servidor HTTP corriendo en el puerto " << port << endl;

    while (true)
    {
        sockaddr_in clientAddr{};
        socklen_t clientSize = sizeof(clientAddr);
        int clientSock = accept(serverSock, (sockaddr *)&clientAddr, &clientSize);
        thread t(handleClient, clientSock);
        t.detach();
    }

    return 0;
}
