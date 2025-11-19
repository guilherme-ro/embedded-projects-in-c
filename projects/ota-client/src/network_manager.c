#include "network_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

// Função de callback para libcurl para armazenar dados baixados na memória
static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    DownloadBuffer *mem = (DownloadBuffer *)userp;

    // Realocar memória para os novos dados
    unsigned char *ptr = realloc(mem->data, mem->size + realsize + 1);
    if (ptr == NULL) {
        fprintf(stderr, "Falha no realloc (out of memory).\n");
        return 0; // Sinaliza erro para libcurl
    }

    mem->data = ptr;
    memcpy(&(mem->data[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->data[mem->size] = 0; // Null-terminator (útil para strings/JSON)

    return realsize;
}

// Função utilitária para download HTTP/HTTPS genérico
static int perform_download(const char *url, DownloadBuffer *buffer) {
    CURL *curl_handle;
    CURLcode res;

    buffer->data = malloc(1); // Inicializa com 1 byte
    buffer->size = 0;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl_handle = curl_easy_init();

    if (curl_handle) {
        curl_easy_setopt(curl_handle, CURLOPT_URL, url);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)buffer);
        curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "ota-client/1.0");
        // Habilitar a verificação de certificados SSL (segurança)
        curl_easy_setopt(curl_handle, CURLOPT_USE_SSL, CURLUSESSL_ALL);
        // Opcional: Definir o CA bundle para sistemas embarcados
        // curl_easy_setopt(curl_handle, CURLOPT_CAINFO, "/etc/ssl/certs/ca-certificates.crt");

        curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0L); // verificação de hostname desabilitada em ambiente de teste
        curl_easy_setopt(curl_handle, CURLOPT_CAINFO, "../cert.pem");
        curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 1L);
        res = curl_easy_perform(curl_handle);

        if (res != CURLE_OK) {
            fprintf(stderr, "download falhou: %s\n", curl_easy_strerror(res));
            free(buffer->data);
            buffer->data = NULL;
            buffer->size = 0;
            return -1;
        }

        curl_easy_cleanup(curl_handle);
    }
    curl_global_cleanup();
    
    return 0;
}

int check_version_availability(const char *url, char **json_response) {
    DownloadBuffer buffer = {0};
    
    if (perform_download(url, &buffer) != 0) {
        *json_response = NULL;
        return -1;
    }

    // Retorna a string JSON (o chamador deve liberar)
    *json_response = (char *)buffer.data;
    
    return 0;
}

int download_firmware(const char *url, DownloadBuffer *buffer) {
    return perform_download(url, buffer);
}
