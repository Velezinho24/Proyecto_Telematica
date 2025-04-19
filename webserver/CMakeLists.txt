cmake_minimum_required(VERSION 3.10)
project(proyecto_http)

set(CMAKE_CXX_STANDARD 17)

# Archivos fuente
add_executable(server
    src/main.cpp
    src/http_server.cpp
    src/http_request.cpp
    src/http_response.cpp
)

# Enlazar bibliotecas necesarias (pthreads para hilos)
target_link_libraries(server pthread)