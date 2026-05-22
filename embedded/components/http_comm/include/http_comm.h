#ifndef HTTP_COMM_H
#define HTTP_COMM_H

#include "esp_err.h"
#include "esp_http_server.h"

typedef char* (*http_get_data_provider_t)(void);

/* Inicializa o motor do servidor HTTP. */
esp_err_t http_server_start(void);

/*
Registra um serviço de um grupo.
uri: caminho do endpoint, ex: "/camera"
method: método HTTP utilizado
queue: fila onde os dados serão colocados
*/
esp_err_t register_POST_endpoint_queue(const char* uri, QueueHandle_t queue);

/*
Registra um serviço de um grupo.
uri: caminho do endpoint, ex: "/camera"
method: método HTTP utilizado
queue: fila onde os dados serão colocados
*/
esp_err_t register_GET_endpoint_callback(const char* uri, http_get_data_provider_t func);

esp_err_t POST_command(const char* ip, const char* path, const char* message);
esp_err_t GET_command(const char* ip, const char* path, char* response_buffer, size_t buffer_len);

#endif