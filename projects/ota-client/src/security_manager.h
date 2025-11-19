#ifndef SECURITY_MANAGER_H
#define SECURITY_MANAGER_H

#include <stddef.h>

// Tamanho esperado do hash SHA256 (256 bits / 8 = 32 bytes)
#define SHA256_HASH_SIZE 32

/**
 * @brief Gera o hash SHA256 de um buffer de dados.
 * * @param data Ponteiro para os dados.
 * @param len Tamanho dos dados.
 * @param output Buffer de saída para o hash (deve ter pelo menos SHA256_HASH_SIZE bytes).
 * @return int 0 em caso de sucesso, -1 em caso de falha.
 */
int generate_sha256_hash(const unsigned char *data, size_t len, unsigned char *output);

/**
 * @brief Verifica se o hash gerado corresponde ao hash esperado.
 * * @param data Ponteiro para os dados do firmware.
 * @param len Tamanho dos dados do firmware.
 * @param expected_hash String hex do hash esperado.
 * @return int 1 se os hashes corresponderem, 0 caso contrário, -1 em caso de erro.
 */
int verify_firmware_integrity(const unsigned char *data, size_t len, const char *expected_hash);

/**
 * @brief Verifica se a assinatura digital corresponde aos dados e à chave pública.
 * @param firmware_data Dados do firmware (payload).
 * @param firmware_len Tamanho dos dados.
 * @param signature Assinatura digital (conteúdo do .sig).
 * @param sig_len Tamanho da assinatura.
 * @param public_key_path Caminho para a chave pública (incorporada no cliente).
 * @return int 1 se a assinatura for válida, 0 caso contrário, -1 em caso de erro.
 */
int verify_firmware_signature(const unsigned char *firmware_data, size_t firmware_len,
                              const unsigned char *signature, size_t sig_len,
                              const char *public_key_path) ;

#endif // SECURITY_MANAGER_H
