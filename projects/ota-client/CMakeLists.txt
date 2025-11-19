# Versão mínima do CMake
cmake_minimum_required(VERSION 3.10)

# Nome do projeto
project(ota-client C)

# Habilitar C com padrões modernos
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Diretório para os arquivos de cabeçalho
include_directories(src)

# --- Dependências ---

# Encontrar libcurl
find_package(CURL REQUIRED)
#find_package(CURL REQUIRED CONFIG)

# Encontrar OpenSSL
find_package(OpenSSL REQUIRED)

# --- Arquivos Fonte ---

# Lista de arquivos fonte
set(SOURCES
    src/main.c
    src/ota_client.c
    src/network_manager.c
    src/security_manager.c
)

# --- Compilação do Executável ---

# Criar o executável 'ota_client_app'
add_executable(ota_client_app ${SOURCES})

# Ligar as bibliotecas necessárias ao executável
target_link_libraries(ota_client_app
    #CURL::CURL          # libcurl
    PRIVATE ${CURL_LIBRARIES} # libcurl módulo FindCURL
    ${OpenSSL_LIBRARIES} # libssl e libcrypto
    ${OPENSSL_CRYPTO_LIBRARY} # Esta é a variável para libcrypto
)

target_link_libraries(ota_client_app PRIVATE ${CURL_LIBRARIES})
