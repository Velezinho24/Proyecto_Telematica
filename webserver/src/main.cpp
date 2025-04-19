#include "http_server.hpp"
#include <iostream>
#include <csignal>

HTTPServer *server = nullptr;

void signal_handler(int signal)
{
    if (server)
    {
        server->stop();
    }
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        std::cerr << "Uso: " << argv[0] << " <HTTP PORT> <Log File> <DocumentRootFolder>" << std::endl;
        return 1;
    }

    int port = std::stoi(argv[1]);
    std::string log_file = argv[2];
    std::string doc_root = argv[3];

    // Registrar manejador de señales para Ctrl+C
    std::signal(SIGINT, signal_handler);

    try
    {
        HTTPServer http_server(port, log_file, doc_root);
        server = &http_server;

        http_server.start();

        // Mantener el programa en ejecución
        while (true)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}