cmake_minimum_required(VERSION 3.10)

# Nome do Projeto
project(TCPEpollServer C)

# Define o padrão C (C99 ou superior, comum em sistemas embarcados)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

# Caminho para o diretório de código fonte
include_directories(src)

# Cria o executável a partir do código fonte
add_executable(tcp_epoll_server src/server.c)

# Liga as bibliotecas de sistema necessárias 
# target_link_libraries(tcp_epoll_server)
