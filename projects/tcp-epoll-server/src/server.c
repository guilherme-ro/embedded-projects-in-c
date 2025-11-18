#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_EVENTS 64
#define BUFFER_SIZE 1024
#define SERVER_PORT 8080

// Função para configurar um descritor de arquivo como não-bloqueante
int set_nonblocking(int fd) {
    int flags;
    if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
        flags = 0;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int main() {
    int listen_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    int epoll_fd;
    struct epoll_event event, events[MAX_EVENTS];
    int n, i;

    printf("Iniciando o servidor TCP (Porta: %d)...\n", SERVER_PORT);

    //Criação do Socket de Escuta (Listening Socket) 
    listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock < 0) {
        perror("Erro ao criar o socket de escuta");
        exit(EXIT_FAILURE);
    }
    
    // Reutilizar endereço (para reinício rápido)
    int optval = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    // Configurar o socket como não-bloqueante 
    if (set_nonblocking(listen_sock) < 0) {
        perror("Erro ao configurar o socket de escuta como não-bloqueante");
        close(listen_sock);
        exit(EXIT_FAILURE);
    }

    // Configuração do Endereço do Servidor 
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // Escuta em todas as interfaces
    server_addr.sin_port = htons(SERVER_PORT);

    // Bind (Associação) do Socket 
    if (bind(listen_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erro no bind");
        close(listen_sock);
        exit(EXIT_FAILURE);
    }

    // Listen (Escuta por Conexões) 
    if (listen(listen_sock, SOMAXCONN) < 0) {
        perror("Erro no listen");
        close(listen_sock);
        exit(EXIT_FAILURE);
    }

    // Criação da Instância epoll 
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("Erro ao criar a instância epoll");
        close(listen_sock);
        exit(EXIT_FAILURE);
    }

    // Adicionar o Socket de Escuta ao epoll
    event.events = EPOLLIN | EPOLLET; // EPOLLIN: Leitura disponível | EPOLLET: Edge Triggered
    event.data.fd = listen_sock;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_sock, &event) == -1) {
        perror("Erro ao adicionar o socket de escuta ao epoll");
        close(listen_sock);
        close(epoll_fd);
        exit(EXIT_FAILURE);
    }

    printf("Servidor escutando na porta %d...\n", SERVER_PORT);

    // Loop Principal do Servidor (I/O Multiplexada) 
    while (1) {
        // Espera por eventos no epoll_fd (bloqueia até que um evento ocorra ou timeout)
        n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (n == -1) {
            if (errno == EINTR) continue; // Interrompido por sinal
            perror("Erro no epoll_wait");
            break;
        }

        // Processa todos os eventos retornados
        for (i = 0; i < n; i++) {
            // Novo Evento no Socket de Escuta (Nova Conexão)
            if (events[i].data.fd == listen_sock) {
                while (1) {
                    client_len = sizeof(client_addr);
                    // Aceita a nova conexão (loop para drenar conexões, característica do EPOLLET)
                    client_sock = accept(listen_sock, (struct sockaddr *)&client_addr, &client_len);
                    
                    if (client_sock == -1) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            // Todas as conexões pendentes foram aceitas
                            break;
                        } else {
                            perror("Erro no accept");
                            break;
                        }
                    }

                    // Configura o novo socket do cliente como não-bloqueante
                    if (set_nonblocking(client_sock) < 0) {
                        perror("Erro ao configurar o socket do cliente como não-bloqueante");
                        close(client_sock);
                        continue;
                    }
                    
                    printf("Nova conexão aceita: FD %d (IP: %s)\n", client_sock, inet_ntoa(client_addr.sin_addr));
                    
                    // Adiciona o novo socket do cliente ao epoll
                    event.events = EPOLLIN | EPOLLET | EPOLLRDHUP; // EPOLLRDHUP para detecção de desconexão
                    event.data.fd = client_sock;
                    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_sock, &event) == -1) {
                        perror("Erro ao adicionar o socket do cliente ao epoll");
                        close(client_sock);
                    }
                }
            }
            // Evento em um Socket de Cliente (Leitura ou Desconexão)
            else {
                int current_fd = events[i].data.fd;

                // Desconexão (O cliente fechou o socket ou erro)
                if (events[i].events & (EPOLLHUP | EPOLLERR | EPOLLRDHUP)) {
                    printf("Conexão fechada ou erro no FD %d.\n", current_fd);
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, current_fd, NULL);
                    close(current_fd);
                    continue;
                }
                
                // Leitura de Dados
                if (events[i].events & EPOLLIN) {
                    char buffer[BUFFER_SIZE];
                    ssize_t bytes_read;
                    
                    // Loop de leitura (característica do EPOLLET: garante que todos os dados pendentes sejam lidos)
                    while ((bytes_read = read(current_fd, buffer, BUFFER_SIZE - 1)) > 0) {
                        buffer[bytes_read] = '\0'; // terminador nulo para printf/strings
                        printf("FD %d: Recebido '%s' (%zd bytes)\n", current_fd, buffer, bytes_read);
                        
                        // Exemplo de Processamento: Ecoar a mensagem de volta
                        ssize_t bytes_sent = write(current_fd, buffer, bytes_read);
                        if (bytes_sent == -1) {
                            perror("Erro ao ecoar dados (write)");
                            break;
                        }
                    }
                    
                    // Trata as condições de saída do loop de leitura
                    if (bytes_read == 0) {
                        // O cliente performou um shutdown ordenado
                        printf("Conexão fechada por peer no FD %d.\n", current_fd);
                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, current_fd, NULL);
                        close(current_fd);
                    } else if (bytes_read == -1 && (errno != EAGAIN && errno != EWOULDBLOCK)) {
                        // Erro real, não apenas sem dados disponíveis
                        perror("Erro ao ler dados (read)");
                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, current_fd, NULL);
                        close(current_fd);
                    }
                }
            }
        }
    }

    // Limpeza (Cleanup) 
    close(listen_sock);
    close(epoll_fd);
    printf("Servidor encerrado.\n");
    return 0;
}
