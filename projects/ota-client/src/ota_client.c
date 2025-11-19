#include "ota_client.h"
#include "network_manager.h"
#include "security_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Configurações de Segurança ---
// Caminho simulado para a chave pública (incorporada no cliente)
#define PUBLIC_KEY_PATH "cert.pem" 

// (Atenção: Para um projeto real, você usaria uma biblioteca JSON robusta como Jansson.
// Aqui, faremos uma análise JSON muito simplificada para manter o código leve).

/**
 * @brief Simplesmente extrai um valor associado a uma chave de uma string JSON (muito frágil, apenas para exemplo).
 * @param json String JSON.
 * @param key Chave a ser procurada.
 * @return char* O valor (string) ou NULL se não encontrado. Deve ser liberado.
 */
static char *simple_json_extract(const char *json, const char *key) {
    char search_key[128];
    // Formato: "chave":
    sprintf(search_key, "\"%s\":", key);
    
    char *start = strstr(json, search_key);
    if (!start) return NULL;

    start += strlen(search_key);

    // Pular espaços e aspas iniciais
    while (*start == ' ' || *start == '\t') start++;
    if (*start == '"') start++;
    
    char *end = start;
    // Procurar pelo terminador da string de valor (aspas, vírgula, ou chave de fechamento)
    while (*end != '"' && *end != ',' && *end != '}' && *end != '\0') end++;
    
    // Alocar e copiar a string extraída
    size_t len = end - start;
    char *value = malloc(len + 1);
    if (!value) return NULL;
    
    strncpy(value, start, len);
    value[len] = '\0';

    return value;
}


int perform_ota_update(const char *version_check_url, const char *current_version) {
    char *response_json = NULL;
    char *new_version = NULL;
    char *firmware_url = NULL;
    char *signature_url = NULL; 
    char *firmware_hash = NULL;
    
    DownloadBuffer firmware_buffer = {0};
    DownloadBuffer signature_buffer = {0}; // Novo buffer para a assinatura
    
    int ret = -1; // Status inicial de falha

    printf("--- Iniciando Cliente OTA (Versão Atual: %s) ---\n", current_version);

    // 1. Verificação de Versão e Obtenção de URLs
    printf("1. Verificando nova versão em: %s\n", version_check_url);

    if (check_version_availability(version_check_url, &response_json) != 0 || response_json == NULL) {
        fprintf(stderr, "Erro ao verificar a disponibilidade da versão.\n");
        goto cleanup;
    }

    // 2. Análise do JSON
    printf("   Resposta do servidor: %s\n", response_json);
    
    new_version = simple_json_extract(response_json, "version");
    firmware_url = simple_json_extract(response_json, "url");
    signature_url = simple_json_extract(response_json, "signature_url"); // Extrai URL da assinatura
    firmware_hash = simple_json_extract(response_json, "hash");

    if (!new_version || !firmware_url || !signature_url || !firmware_hash) {
        fprintf(stderr, "Erro: Falha ao analisar JSON (version, url, signature_url ou hash ausentes).\n");
        goto cleanup;
    }

    // 3. Comparação de Versão
    if (strcmp(new_version, current_version) <= 0) {
        printf("☑️ Versão atual (%s) é a mais recente. Nenhuma atualização necessária.\n", current_version);
        ret = 0;
        goto cleanup;
    }

    printf("2. Nova versão disponível: %s\n", new_version);
    printf("   URL de download do Firmware: %s\n", firmware_url);
    printf("   URL de download da Assinatura: %s\n", signature_url);

    // 4. Download do Firmware
    printf("3. Baixando o firmware...\n");
    if (download_firmware(firmware_url, &firmware_buffer) != 0) {
        fprintf(stderr, "Erro ao baixar o firmware.\n");
        goto cleanup;
    }
    printf("   Download do Firmware concluído. Tamanho: %zu bytes.\n", firmware_buffer.size);
    
    // 5. Download da Assinatura
    printf("4. Baixando a assinatura digital...\n");
    if (download_firmware(signature_url, &signature_buffer) != 0) { // Reutiliza a função de download
        fprintf(stderr, "Erro ao baixar a assinatura.\n");
        goto cleanup;
    }
    printf("   Download da Assinatura concluído. Tamanho: %zu bytes.\n", signature_buffer.size);

    // 6. Verificação de Integridade (SHA256)
    printf("5. Verificando integridade (SHA256)... Hash esperado: %s\n", firmware_hash);
    if (verify_firmware_integrity(firmware_buffer.data, firmware_buffer.size, firmware_hash) != 1) {
        fprintf(stderr, "❌ Falha na verificação de integridade SHA256. Atualização abortada.\n");
        goto cleanup;
    }

    // 7. Verificação de Autenticidade (Assinatura Digital)
    printf("6. Verificando autenticidade (Assinatura Digital - %s)...", PUBLIC_KEY_PATH);
    
    // Chamada à função de segurança aprimorada (OpenSSL)
    // OBS: Esta função precisa ser implementada em security_manager.c
    if (verify_firmware_signature(
        firmware_buffer.data, firmware_buffer.size, 
        signature_buffer.data, signature_buffer.size, 
        PUBLIC_KEY_PATH) != 1) 
    {
        fprintf(stderr, "❌ Falha na autenticação da assinatura digital. Atualização abortada.\n");
        goto cleanup;
    }
    
    // 8. Aplicação da Atualização
    printf("7. Atualização segura e autêntica. Aplicando o novo firmware...\n");
    printf("   [SIMULAÇÃO] Firmware escrito com sucesso em /dev/fw_partition...\n");
    printf("   [SIMULAÇÃO] Sistema será reiniciado para Versão %s...\n", new_version);

    ret = 0; // Sucesso
    
cleanup:
    // Limpeza de buffers
    if (firmware_buffer.data) {
        free(firmware_buffer.data);
    }
    if (signature_buffer.data) {
        free(signature_buffer.data);
    }

    // Limpeza de strings alocadas (funções json_extract e network_manager)
    if (response_json) free(response_json);
    if (new_version) free(new_version);
    if (firmware_url) free(firmware_url);
    if (signature_url) free(signature_url);
    if (firmware_hash) free(firmware_hash);

    printf("--- Processo OTA Concluído ---\n");

    return ret;
}