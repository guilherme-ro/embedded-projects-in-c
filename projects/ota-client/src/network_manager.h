#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <stddef.h>

// Estrutura para manter o buffer de dados baixados e seu tamanho
typedef struct {
    unsigned char *data;
    size_t size;
} DownloadBuffer;

/**
 * @brief Verifica se uma nova versão está disponível.
 * * @param url URL do endpoint de verificação de versão (retorna JSON, ex: {"version": "1.1.0", "url": "...", "hash": "..."}).
 * @param json_response Ponteiro para onde armazenar a string de resposta JSON. Deve ser liberado pelo chamador.
 * @return int 0 em caso de sucesso, -1 em caso de falha.
 */
int check_version_availability(const char *url, char **json_response);

/**
 * @brief Baixa um arquivo de firmware de uma URL.
 * * @param url URL direta do arquivo de firmware.
 * @param buffer Ponteiro para a estrutura DownloadBuffer para armazenar os dados.
 * @return int 0 em caso de sucesso, -1 em caso de falha.
 */
int download_firmware(const char *url, DownloadBuffer *buffer);

#endif // NETWORK_MANAGER_H
