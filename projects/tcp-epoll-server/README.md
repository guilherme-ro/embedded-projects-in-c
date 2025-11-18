## 💡 Servidor TCP de Alto Desempenho (epoll)

Este projeto implementa um **Servidor TCP** simples em Linguagem C para Linux, utilizando uma técnica avançada chamada **I/O Multiplexada** com `epoll`.

### O que é este projeto?

É um programa de servidor que fica aguardando e gerenciando múltiplas conexões de clientes (como navegadores ou programas de teste) em uma única porta (padrão: **8080**).

O servidor implementa um serviço simples de **"Echo"**: qualquer mensagem que um cliente envia, o servidor envia de volta.

### Por que usar `epoll`?

Em servidores tradicionais, geralmente você tem que usar uma _thread_ ou um processo separado para lidar com cada cliente. Se você tiver 10.000 clientes, você precisa de 10.000 _threads_. Isso consome muita memória e recursos, sendo ineficiente.

O `epoll` resolve isso:

1.  Ele permite que **uma única _thread_** observe milhares de _sockets_ (conexões).
    
2.  O servidor **bloqueia** e espera apenas no `epoll`.
    
3.  Quando **dados chegam** ou uma **nova conexão** ocorre em qualquer um desses _sockets_, o `epoll` _acorda_ a _thread_ e diz: "Ei, estes aqui estão prontos para você!"
    
4.  A _thread_ processa apenas o que está pronto.
    

Essa abordagem, conhecida como **I/O Multiplexada**, é crucial para construir **sistemas embarcados** e servidores web que precisam ser **rápidos** e **economizar recursos**.

----------

## 🛠️ Tecnologias Utilizadas

-   **Linguagem C:** A base de tudo, usando a API de _sockets_ do Linux.
    
-   **`epoll`:** O mecanismo de I/O eficiente do Kernel Linux (substituindo o antigo e lento `select`/`poll`).
    
-   **Sockets Não-Bloqueantes:** Permite que as operações de leitura (`read`) e aceitação (`accept`) falhem imediatamente se não houver dados, impedindo que a única _thread_ fique travada esperando por um único cliente.
    
-   **CMake:** O sistema de _build_  usado para compilar o código de forma simples em qualquer ambiente Linux.
    

----------

## 🚀 Como Rodar o Projeto

Para compilar e executar o servidor no Linux:

### 1. Pré-requisitos

Ter `git`, `cmake` e um compilador C (como `gcc`) instalados.

### 2. Clonar o Repositório

Bash

```
git clone https://github.com/SeuUsuario/tcp_epoll_server.git # Substitua pelo seu link
cd tcp_epoll_server

```

### 3. Compilação (Build)

Usamos o CMake para gerar os arquivos de compilação.

Bash

```
mkdir build
cd build
cmake ..     # Gera os arquivos de build
make         # Compila o executável

```

### 4. Executar o Servidor

O executável estará dentro do diretório `build`.

Bash

```
./tcp_epoll_server

```

O servidor será iniciado na porta **8080**.

### 5. Testar a Conexão

Abra uma ou mais janelas de terminal separadas e use o `netcat` (`nc`) ou `telnet` para conectar:

Bash

```
nc 127.0.0.1 8080

```

-   Digite uma mensagem e pressione **Enter**.
    
-   Você verá o servidor enviar a mensagem de volta (Echo).
    
-   Você também verá o log de eventos no terminal do servidor.
