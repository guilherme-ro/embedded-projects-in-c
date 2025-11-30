# 🤖 Sistema de Monitoramento de Atuadores (Simulação Embarcada)

## # 💡 O que é este projeto?

Este projeto é uma **simulação em C** de um sistema de controle e monitoramento de dois atuadores (como um motor e uma válvula solenoide), focado em aplicar as melhores práticas de **otimização de memória** e **organização de código** essenciais para **sistemas embarcados**1.

O código demonstra:

-   O uso de **tipos de dados de largura fixa** (`stdint.h`) para garantir portabilidade e otimização de memória2.
    
-   Organização de dados através de **estruturas (`struct`)** e definição de estados via **enumeração**.
    
-   Implementação de funções de controle que manipulam estruturas via **ponteiros** (passagem por referência) para evitar cópias desnecessárias de dados.
    
-   Uso do qualificador **`volatile`** para variáveis globais de tempo, simulando interrupções.
    
-   Lógica básica de monitoramento de feedback, incluindo detecção de falha e mudança de estado.
    

----------

## 📁 Estrutura do Projeto

A estrutura de arquivos sugerida para o projeto é a seguinte:

```
sistema-monitoramento-atuadores/
├── CMakeLists.txt
├── Readme.md
└── src/
    └── atuator_monitor.c

```

----------

## 🛠️ Tecnologias Utilizadas

-   **Linguagem C:** Linguagem principal para desenvolvimento.
    
-   `stdint.h`: Biblioteca fundamental para tipos de dados de largura fixa (`uint8_t`, `int16_t`, etc.)

-   **CMake**: Sistema de build moderno e _cross-platform_ utilizado para compilar o código em ambientes Linux/Unix.

-   **GCC/Clang**: Compilador C (necessário para o processo de build).


----------

## 🚀 Como Rodar o Projeto

### Pré-requisitos

Você deve ter o **CMake** e um compilador C (como o **GCC**) instalados em seu sistema (ambiente Linux é o recomendado).

### Para compilar e executar no Linux:

Siga os passos abaixo para configurar, compilar e rodar a simulação:

1.  Crie o diretório de build:
    
    Crie um diretório para manter os arquivos de build separados do código-fonte.
    
    Bash
    
    ```
    mkdir build
    cd build
    
    ```
    
2.  Configure o Projeto com CMake:
    
    O CMake lerá o arquivo CMakeLists.txt e gerará os Makefiles.
    
    Bash
    
    ```
    cmake ..
    
    ```
    
3.  Compile o Executável:
    
    Use o make para compilar o código. Isso gerará o binário actuator_monitor.
    
    Bash
    
    ```
    make
    
    ```
    
4.  Execute a Simulação:
    
    Rode o binário para iniciar a simulação do ciclo de vida dos atuadores.
    
    Bash
    
    ```
    ./actuator_monitor
    
    ```
    

#### Exemplo de Saída Esperada:

```
>>> INICIALIZAÇÃO DO SISTEMA <<<
Atuador 1 (Pino 4) inicializado para OCIOSO.
Atuador 2 (Pino 8) inicializado para OCIOSO.

>>> INÍCIO DA SIMULAÇÃO DO CICLO DE VIDA <<<
--- Atuador 1 ATIVADO no tempo: 1000 ms. Simulação de escrita em hardware: Pino 4 LIGADO.

* Tempo simulado avança para 1500 ms.
  - Atuador 2 feedback: 500 (Limite de falha: 1000).
  - Atuador 1 feedback: 1200 (Limite de falha: 1000).
  *** ATENÇÃO: Atuador 1 entrou em estado de FALHA! ***

* Tentando ativar a Válvula no tempo 1700 ms.
--- Atuador 2 ATIVADO no tempo: 1700 ms. Simulação de escrita em hardware: Pino 8 LIGADO.

>>> FIM DA SIMULAÇÃO: STATUS FINAIS <<<

--- STATUS FINAL Atuador 1 ---
  ID: 1
  Pino de Controle: 4
  Estado: FALHA
  Tempo de Ativação (ms): 1000
  Última Leitura: 1200
-------------------------------

--- STATUS FINAL Atuador 2 ---
  ID: 2
  Pino de Controle: 8
  Estado: ATIVO
  Tempo de Ativação (ms): 1700
  Última Leitura: 500
-------------------------------
```
