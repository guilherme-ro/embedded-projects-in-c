#ifndef OTA_CLIENT_H
#define OTA_CLIENT_H

/**
 * @brief Executa o processo completo de verificação e atualização OTA.
 * * @param version_check_url URL para verificar a nova versão (JSON).
 * @param current_version Versão atual do firmware.
 * @return int 0 em caso de sucesso (atualização concluída ou não necessária), -1 em caso de falha.
 */
int perform_ota_update(const char *version_check_url, const char *current_version);

#endif // OTA_CLIENT_H
