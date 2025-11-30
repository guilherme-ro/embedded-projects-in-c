# Versão mínima do CMake
cmake_minimum_required(VERSION 3.10)

# Define o nome do projeto
project(AtuatorMonitor C)

# Define as opções de compilação
# C99 é um padrão comum em sistemas embarcados
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

# Caminho para o diretório de código fonte
include_directories(src)

# Define opções de otimização e warnings para desenvolvimento embarcado
# -Wall: Habilita todos os warnings comuns
# -Wextra: Habilita warnings adicionais
# -O2: Nível de otimização moderado (para simulação, mas útil para embarcados)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -O2")

# Adiciona o executável
# O executável será chamado 'atuator_monitor' e será construído a partir do arquivo 'atuator_monitor.c'
add_executable(atuator_monitor src/atuator_monitor.c)
