#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// Definição da enumeração para os estados operacionais do atuador 
typedef enum {
    OCIOSO, // 0
    ATIVO,  // 1
    FALHA   // 2
} ESTADO_ATUADOR;

// Definição da estrutura Atuador 
// Todos os campos utilizam tipos de dados de largura fixa (<stdint.h>)
typedef struct {
    uint8_t id_atuador;          // 8 bits, sem sinal 
    uint8_t pino_controle;       // 8 bits, sem sinal 
    ESTADO_ATUADOR estado_atual; // Tipo enum definido acima 
    uint32_t tempo_ativacao_ms;  // 32 bits, sem sinal (tempo de ativação) 
    int16_t valor_leitura;       // 16 bits, com sinal (feedback de leitura) 
} Atuador;

// Variável global para simular o tempo em milissegundos 
// O qualificador 'volatile' indica que a variável pode ser alterada por fatores externos (como uma ISR),
// evitando otimizações indevidas pelo compilador.
volatile uint32_t tempo_simulado_ms = 0;

// Limite de feedback para detecção de falha 
#define LIMITE_FALHA 1000

// Inicializa a estrutura do atuador
// Usa ponteiro para modificar a estrutura original (passagem por referência) 
void inicializa_atuador(Atuador *a, uint8_t id, uint8_t pino) {
    if (a != NULL) {
        a->id_atuador = id;
        a->pino_controle = pino;
        a->estado_atual = OCIOSO; // Define o estado inicial como OCIOSO 
        a->tempo_ativacao_ms = 0;
        a->valor_leitura = 0;
        printf("Atuador %u (Pino %u) inicializado para OCIOSO.\n", a->id_atuador, a->pino_controle);
    }
}

// Ativa o atuador 
// Usa ponteiro para modificar a estrutura original (passagem por referência) 
void ativa_atuador(Atuador *a, volatile uint32_t tempo_atual) {
    if (a != NULL) {
        if (a->estado_atual == OCIOSO) {
            a->estado_atual = ATIVO; // Muda o estado para ATIVO 
            a->tempo_ativacao_ms = tempo_atual; // Armazena o tempo de ativação 
            printf("--- Atuador %u ATIVADO no tempo: %lu ms. Simulação de escrita em hardware: Pino %u LIGADO.\n",
                   a->id_atuador, (unsigned long)tempo_atual, a->pino_controle);
        } else if (a->estado_atual == FALHA) {
            printf("Atuador %u em FALHA. Não pode ser ativado.\n", a->id_atuador);
        } else {
             printf("Atuador %u já está ATIVO.\n", a->id_atuador);
        }
    }
}

// Processa o feedback de leitura do atuador 
// Usa ponteiro para modificar a estrutura original (passagem por referência) 
void processa_feedback(Atuador *a, int16_t leitura_simulada) {
    if (a != NULL) {
        a->valor_leitura = leitura_simulada; // Atualiza o valor_leitura 

        printf("  - Atuador %u feedback: %d (Limite de falha: %d).\n",
               a->id_atuador, a->valor_leitura, LIMITE_FALHA);

        // Verifica se a leitura excede o limite, mudando o estado para FALHA 
        if (a->valor_leitura > LIMITE_FALHA) {
            a->estado_atual = FALHA;
            printf("  *** ATENÇÃO: Atuador %u entrou em estado de FALHA! ***\n", a->id_atuador);
        } else if (a->estado_atual != FALHA) {
            // Se o atuador não estiver em falha, ele permanece ATIVO ou OCIOSO (se não foi ativado)
            // Em uma implementação real, a verificação de OCIOSO seria mais complexa.
            // Para a simulação, se a leitura está OK e não está em FALHA, mantemos o estado.
        }
    }
}

// Função auxiliar para imprimir o status final 
void imprime_status(const Atuador *a) {
    if (a != NULL) {
        const char *estado_str;
        // Mapeamento do enum para string para impressão
        switch (a->estado_atual) {
            case OCIOSO: estado_str = "OCIOSO"; break;
            case ATIVO:  estado_str = "ATIVO"; break;
            case FALHA:  estado_str = "FALHA"; break;
            default:     estado_str = "DESCONHECIDO";
        }

        printf("\n--- STATUS FINAL Atuador %u ---\n", a->id_atuador);
        printf("  ID: %u\n", a->id_atuador);
        printf("  Pino de Controle: %u\n", a->pino_controle);
        printf("  Estado: %s\n", estado_str);
        printf("  Tempo de Ativação (ms): %lu\n", (unsigned long)a->tempo_ativacao_ms);
        printf("  Última Leitura: %d\n", a->valor_leitura);
        printf("-------------------------------\n");
    }
}


// função principal
int main() {
    // Cria duas instâncias da estrutura Atuador (motor e valvula) 
    Atuador motor;
    Atuador valvula;

    printf(">>> INICIALIZAÇÃO DO SISTEMA <<<\n");

    // Inicializa as duas estruturas 
    inicializa_atuador(&motor, 1, 4);    // Motor: ID 1, Pino 4
    inicializa_atuador(&valvula, 2, 8);  // Válvula: ID 2, Pino 8

    printf("\n>>> INÍCIO DA SIMULAÇÃO DO CICLO DE VIDA <<<\n");

    // Ativa o motor 
    tempo_simulado_ms = 1000; // Tempo simulado = 1000ms 
    ativa_atuador(&motor, tempo_simulado_ms);

    // Simula a passagem do tempo 
    tempo_simulado_ms += 500; // Tempo atual = 1500ms
    printf("\n* Tempo simulado avança para %lu ms.\n", (unsigned long)tempo_simulado_ms);

    // Executa processa_feedback para ambos 
    // Válvula: Leitura de sucesso (500) 
    processa_feedback(&valvula, 500);

    // Motor: Leitura de falha (1200) 
    processa_feedback(&motor, 1200);

    // Tenta ativar a válvula, que agora está OCIOSA 
    tempo_simulado_ms += 200; // Tempo atual = 1700ms
    printf("\n* Tentando ativar a Válvula no tempo %lu ms.\n", (unsigned long)tempo_simulado_ms);
    ativa_atuador(&valvula, tempo_simulado_ms);

    // Imprime o status final das duas estruturas 
    printf("\n>>> FIM DA SIMULAÇÃO: STATUS FINAIS <<<\n");
    imprime_status(&motor);
    imprime_status(&valvula);

    return 0;
}
