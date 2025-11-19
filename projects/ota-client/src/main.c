#include "ota_client.h"
#include <stdio.h>
#include <stdlib.h>

// Para testes, o servidor precisa expor um endpoint que retorne um JSON no formato:
// {"version": "1.1.0", "url": "https://seu-servidor.com/firmware/v1.1.0.bin", "hash": "a228f411..."}
// E um arquivo binário de firmware na URL.

#define CURRENT_FIRMWARE_VERSION "1.0.0" 
// URL de exemplo (use HTTPS para segurança real)
#define VERSION_CHECK_ENDPOINT "https://127.0.0.1:8443/api/firmware/latest"

int main() {
    printf("Cliente de Atualização OTA para Linux Embarcado\n");
    printf("----------------------------------------------\n");

    // Exemplo de uso:
    int result = perform_ota_update(VERSION_CHECK_ENDPOINT, CURRENT_FIRMWARE_VERSION);

    if (result == 0) {
        printf("Resultado final: SUCESSO.\n");
    } else {
        fprintf(stderr, "Resultado final: FALHA.\n");
    }

    return result;
}
