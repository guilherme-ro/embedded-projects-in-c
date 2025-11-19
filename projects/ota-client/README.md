# 💡 OTA Client: Sistema de Atualização de Firmware Seguro (Over-The-Air)

Este projeto demonstra um cliente em **Linguagem C** projetado para sistemas **Linux Embarcados**, capaz de realizar atualizações de firmware de forma segura via HTTPS. Ele implementa comunicação de rede robusta e um fluxo de segurança em duas camadas (Integridade SHA256 e Autenticidade por Assinatura Digital RSA/OpenSSL).

O projeto é construído usando **CMake** como sistema de _build_ modular.

----------

## 📁 Estrutura do Projeto

A organização modular do código separa as responsabilidades (Rede, Segurança e Lógica Principal), facilitando a manutenção e a portabilidade.

```
ota-client/
├── build/                 <-- Diretório criado pelo CMake (artefatos de build)
├── src/
│   ├── main.c             // Ponto de entrada e versão atual do firmware.
│   ├── ota_client.c       // Lógica principal: gerencia o fluxo de verificação e aplicação.
│   ├── ota_client.h       
│   ├── network_manager.c  // Comunicação HTTP/HTTPS (usa libcurl).
│   ├── network_manager.h  
│   └── security_manager.c // Funções de criptografia e verificação (usa OpenSSL).
│   └── security_manager.h 
├── CMakeLists.txt         // Sistema de build moderno.
├── firmware_v1.1.0.bin    // [Artefato] Arquivo de firmware de exemplo.
├── firmware_v1.1.0.sig    // [Artefato] Assinatura digital VÁLIDA do firmware.
├── cert.pem               // [Artefato] Chave Pública/Certificado SSL (CA confiável pelo cliente).
└── key.pem                // [Artefato] Chave Privada (usada apenas pelo desenvolvedor para assinar).

```

----------

## 🛠️ Tecnologias Utilizadas

-   **Linguagem C:** Linguagem principal, ideal para sistemas embarcados devido à eficiência e controle de memória.
    
-   **CMake**: Ferramenta de _meta-build_ para configurar e gerar arquivos de build portáveis (makefiles, etc.).
    
-   **libcurl**: Biblioteca robusta para transferências de dados via protocolos de rede (HTTP/HTTPS).
    
-   **OpenSSL (libssl/libcrypto)**: Utilizada para cálculo de hash (SHA256) e verificação de assinatura digital (RSA).

-   **Python 3 (`http.server`, `ssl`)**: Usado para simular o **Servidor Mock OTA** (Server-Side), servindo o firmware e a assinatura via **HTTPS**.

----------

## 🛡️ Fluxo de Execução e Segurança

O fluxo de atualização é estritamente sequencial, garantindo que o firmware só seja aplicado se passar por ambas as camadas de segurança (Integridade e Autenticidade).

### 1. Download do Firmware e da Assinatura

O cliente primeiro verifica a versão e, se houver uma atualização disponível, baixa o **arquivo binário do firmware** e, separadamente, o **arquivo de assinatura digital (.sig)** correspondente. Ambos são armazenados temporariamente na memória (RAM).

### 2. Verificação de Integridade (SHA256)

**Conceito:** A Integridade garante que o arquivo não foi corrompido durante o download ou por falhas no armazenamento.

-   O cliente calcula o **Hash SHA256** dos dados do firmware baixado.
    
-   Compara o hash calculado com o hash **esperado**, fornecido no JSON pelo servidor.
    
-   Se os hashes não coincidirem, o download é rejeitado.
    

### 3. Download da Assinatura VÁLIDA

**Conceito:** A Assinatura Válida é o resultado de uma operação criptográfica. É um pequeno bloco de dados gerado pela **Chave Privada** do desenvolvedor. Se a assinatura for válida, ela prova que **apenas a pessoa com a Chave Privada** (o fabricante) criou e aprovou o firmware.

-   O cliente baixa este artefato binário, que será usado no próximo passo.
    

### 4. Verificação de Autenticidade (Assinatura Digital/OpenSSL)

**Conceito:** A Autenticidade garante que o firmware é genuíno e foi assinado pelo fabricante. É a principal defesa contra _malware_ e _firmware_ falsificado.

-   O cliente utiliza a **Chave Pública** (embutida no cliente, no arquivo `cert.pem`) para verificar se a Assinatura (baixada no passo anterior) corresponde ao Hash (calculado no passo 2).
    
-   Se a Chave Pública conseguir validar a assinatura, a autenticidade é confirmada.
    

Se **ambas** as verificações (Integridade e Autenticidade) forem bem-sucedidas, o firmware é aplicado.

----------

## 🚀 Como Executar o Projeto

Para testar o sistema completo, você precisa do Cliente C compilado e do Servidor Mock Python rodando com os artefatos de segurança corretos.

### 4.1. Configuração e Geração de Artefatos de Segurança

Execute os comandos abaixo na pasta raiz do projeto (`ota-client/`).

#### A. Gerar o Certificado SSL (Chave Pública/Privada)

Isso criará o certificado (`cert.pem`) e a chave privada (`key.pem`) no formato PEM. A **Chave Pública** no `cert.pem` será usada pelo cliente C para autenticar o servidor HTTPS e a assinatura do firmware.

Bash

```
# Gera key.pem (chave privada) e cert.pem (certificado auto-assinado)
openssl req -newkey rsa:2048 -nodes -keyout key.pem -x509 -days 365 -out cert.pem

```

> **NOTA:** Quando solicitado, o campo **Common Name** deve ser definido como **`localhost`** ou **`127.0.0.1`** para evitar erros na verificação de hostname (que foi desabilitada no código C para testes locais).

#### B. Gerar o Firmware e a Assinatura

Foi gerado um arquivo binário fictício e, em seguida, foi criada uma assinatura digital válida usando a **Chave Privada** (`key.pem`).

Bash

```
# 1. Cria o arquivo de firmware fictício (payload)
echo "This is the content of the new firmware version 1.1.0" > firmware_v1.1.0.bin

# 2. Calcula o Hash SHA256 do firmware (usado para verificar a integridade)
openssl dgst -sha256 -binary firmware_v1.1.0.bin > firmware_v1.1.0.hash

# 3. Gera a Assinatura Digital (o conteúdo do .sig) usando a Chave Privada
# Isso assina o hash do firmware.
openssl pkeyutl -sign -in firmware_v1.1.0.hash -inkey key.pem -pkeyopt digest:sha256 -out firmware_v1.1.0.sig

```

### 4.2. Compilando o Cliente C (CMake)

 `gcc`, `cmake`, `libcurl4-openssl-dev` e `libssl-dev` devem estar instalados.

Bash

```
# 1. Entra no diretório raiz do projeto
cd ota-client

# 2. Cria e entra no diretório de build
mkdir build
cd build

# 3. Configura o projeto (encontra libcurl e OpenSSL)
cmake ..

# 4. Compila o executável
make

# O executável compilado será: ota_client_app

```

### 4.3. Rodando o Servidor Mock Python

O script do servidor deve ser executado na pasta raiz do projeto, onde os arquivos `.bin`, `.sig`, `cert.pem` e `key.pem` estão localizados.

Bash

```
# Na pasta raiz do projeto (ota-client/)
python3 MockOTAServer.py

```

O servidor será iniciado na porta **8443** (HTTPS).

### 4.4. Executando o Cliente OTA

Com o servidor rodando em uma janela de terminal, execute o cliente C em outra janela.

Bash

```
# Na pasta de build (ou mova o binário para a raiz, alterando o caminho de cert.pem em network_manager.c antes de compilar o cliente ota)
./build/ota_client_app

```

O cliente irá se conectar via HTTPS, baixar o firmware e a assinatura, e as verificações de integridade (SHA256) e autenticidade (OpenSSL) devem ser concluídas com sucesso.
