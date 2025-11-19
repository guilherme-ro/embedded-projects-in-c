#include "security_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/x509.h>
#include <openssl/pem.h>

/**
 * Converte uma string hexadecimal em um array de bytes.
 * @param hex_str A string hexadecimal (ex: "a1b2c3d4...").
 * @param byte_array O array de bytes de saída.
 * @param len O tamanho (em bytes) do array de saída.
 */
static void hex_to_bytes(const char *hex_str, unsigned char *byte_array, size_t len) {
    for (size_t i = 0; i < len; i++) {
        sscanf(hex_str + 2 * i, "%2hhx", &byte_array[i]);
    }
}

int generate_sha256_hash(const unsigned char *data, size_t len, unsigned char *output) {
    if (!SHA256(data, len, output)) {
        fprintf(stderr, "Erro ao gerar hash SHA256.\n");
        return -1;
    }
    return 0;
}

int verify_firmware_integrity(const unsigned char *data, size_t len, const char *expected_hash) {
    unsigned char generated_hash[SHA256_HASH_SIZE];
    unsigned char expected_hash_bytes[SHA256_HASH_SIZE];

    // 1. Gerar o hash do firmware
    if (generate_sha256_hash(data, len, generated_hash) != 0) {
        return -1; // Erro na geração do hash
    }

    // 2. Converter o hash esperado de string hex para bytes
    if (strlen(expected_hash) != SHA256_HASH_SIZE * 2) {
        fprintf(stderr, "Erro: Tamanho do hash esperado é inválido.\n");
        return -1;
    }
    hex_to_bytes(expected_hash, expected_hash_bytes, SHA256_HASH_SIZE);

    // 3. Comparar os hashes
    if (memcmp(generated_hash, expected_hash_bytes, SHA256_HASH_SIZE) == 0) {
        printf("✅ Integridade verificada: Hashes correspondem.\n");
        return 1;
    } else {
        printf("❌ Falha na verificação de integridade: Hashes NÃO correspondem.\n");
        // Opcional: imprimir os hashes para debug
        printf("   Gerado: ");
        for (int i = 0; i < SHA256_HASH_SIZE; i++) printf("%02x", generated_hash[i]);
        printf("\n");
        printf("   Esperado: %s\n", expected_hash);
        return 0;
    }
}

int verify_firmware_signature(const unsigned char *firmware_data, size_t firmware_len,
                              const unsigned char *signature, size_t sig_len,
                              const char *public_key_path) 
{
    FILE *fp = NULL;
    EVP_PKEY *pub_key = NULL;
    EVP_MD_CTX *md_ctx = NULL;
    int ret = 0; // Assume falha

    // 1. Carregar a Chave Pública (do arquivo cert.pem)
    fp = fopen(public_key_path, "r");
    if (!fp) {
        fprintf(stderr, "Erro de segurança: Não foi possível abrir a chave pública (%s).\n", public_key_path);
        return -1;
    }
    
    // Ler a chave pública a partir do arquivo PEM/Certificado
    // Usamos PEM_read_PUBKEY ou PEM_read_X509, dependendo do formato.
    // Como geramos um certificado X509 (cert.pem), vamos extrair a chave pública dele.
    X509 *cert = PEM_read_X509(fp, NULL, NULL, NULL);
    fclose(fp); // Fecha o arquivo assim que a leitura terminar
    
    if (!cert) {
        fprintf(stderr, "Erro de segurança: Falha ao ler certificado X509.\n");
        return -1;
    }

    pub_key = X509_get_pubkey(cert);
    if (!pub_key) {
        fprintf(stderr, "Erro de segurança: Falha ao extrair chave pública do certificado.\n");
        X509_free(cert);
        return -1;
    }

    // 2. Configurar o contexto de verificação (usando SHA256)
    md_ctx = EVP_MD_CTX_new();
    if (!md_ctx) { goto cleanup; }

    // 3. Inicializar a verificação (RSA/SHA256)
    if (EVP_DigestVerifyInit(md_ctx, NULL, EVP_sha256(), NULL, pub_key) != 1) { goto cleanup; }

    // 4. Alimentar os dados (payload do firmware)
    if (EVP_DigestVerifyUpdate(md_ctx, firmware_data, firmware_len) != 1) { goto cleanup; }

    // 5. Verificar a Assinatura (compara o que foi calculado com o conteúdo do signature_buffer)
    ret = EVP_DigestVerifyFinal(md_ctx, signature, sig_len); 

    if (ret == 1) {
        printf("✅ Assinatura digital válida. Autenticidade confirmada.\n");
    } else if (ret == 0) {
        // Retorno 0 significa que a assinatura NÃO CORRESPONDE, mas o processo foi concluído.
        printf("❌ Assinatura digital inválida.\n");
    } else {
        // Retorno -1 significa um erro interno (Ex: EVP_DigestVerifyFinal falhou)
        fprintf(stderr, "Erro de segurança: Erro interno durante a verificação da assinatura.\n");
    }
    
cleanup:
    if (md_ctx) EVP_MD_CTX_free(md_ctx);
    if (pub_key) EVP_PKEY_free(pub_key);
    if (cert) X509_free(cert);
    
    return ret == 1 ? 1 : (ret == 0 ? 0 : -1);
}