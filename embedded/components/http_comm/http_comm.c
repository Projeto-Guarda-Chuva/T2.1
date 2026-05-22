#include "http_comm.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "HTTP_COMM";
static httpd_handle_t server_handle = NULL;


esp_err_t http_server_start(void) {
    if (server_handle != NULL) return ESP_OK;

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    // config.stack_size = 8192; // descomentar caso o tamanho da stack seja insuficiente para os handlers
    // config.lru_purge_enable = true; // Libera conexões antigas se lotar

    ESP_LOGI(TAG, "Iniciando Servidor na porta %d...", config.server_port);
    esp_err_t ret = httpd_start(&server_handle, &config);
    
    return ret;
}


static esp_err_t generic_post_handler(httpd_req_t *req) {
    size_t length = req->content_len;
    
    char *buf = malloc(length + 1);
    if (buf == NULL) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "RAM LIMITATION");
        return ESP_FAIL;
    }

    int ret = httpd_req_recv(req, buf, length);
    if (ret <= 0) {
        free(buf);
        return ESP_FAIL;
    }
    buf[ret] = '\0';

    QueueHandle_t queue = (QueueHandle_t) req->user_ctx;
    if (xQueueSend(queue, &buf, 0) != pdPASS) {
        free(buf);
    }

    httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}


static esp_err_t generic_get_handler(httpd_req_t *req) {
    http_get_data_provider_t provider = (http_get_data_provider_t) req->user_ctx;

    char *buffer_response = provider();
    if (buffer_response == NULL) {
        return httpd_resp_send_500(req);
    }

    esp_err_t res = httpd_resp_send(req, buffer_response, HTTPD_RESP_USE_STRLEN);
    free(buffer_response);

    return res;
}

esp_err_t register_POST_endpoint_queue(const char* uri, QueueHandle_t queue) {
    if (server_handle == NULL) {
        ESP_LOGE(TAG, "Servidor nao iniciado!");
        return ESP_ERR_INVALID_STATE;
    }

    httpd_uri_t config = {
        .uri      = uri,
        .method   = HTTP_POST,
        .handler  = generic_post_handler,
        .user_ctx = queue
    };

    ESP_LOGI(TAG, "POST endpoint registrado: %s", uri);
    return httpd_register_uri_handler(server_handle, &config);
}


esp_err_t register_GET_endpoint_callback(const char* uri, http_get_data_provider_t func) {
    if (server_handle == NULL) {
        ESP_LOGE(TAG, "Servidor nao iniciado!");
        return ESP_ERR_INVALID_STATE;
    }
    
    httpd_uri_t config = {
        .uri = uri,
        .method = HTTP_GET,
        .handler = generic_get_handler,
        .user_ctx = func
    };

    ESP_LOGI(TAG, "GET endpoint registrado: %s", uri);
    return httpd_register_uri_handler(server_handle, &config);
}


esp_err_t POST_command(const char* ip, const char* path, const char* message) {
    char url[128];
    snprintf(url, sizeof(url), "http://%s%s", ip, path);

    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_POST,
        .timeout_ms = 1500,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) 
        return ESP_FAIL;

    esp_http_client_set_post_field(client, message, strlen(message));
    esp_http_client_set_header(client, "Content-Type", "text/plain");

    esp_err_t err = esp_http_client_perform(client);
    
    if (err == ESP_OK) {
        ESP_LOGI("HTTP_POST", "Comando enviado. Status: %d", esp_http_client_get_status_code(client));
    }

    esp_http_client_cleanup(client);
    return err;
}

esp_err_t POST_image(const char* ip, const char* path, const uint8_t* image_data, size_t image_size) {
    char url[128];
    snprintf(url, sizeof(url), "http://%s%s", ip, path);

    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_POST,
        .timeout_ms = 3000,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    if (!client)
        return ESP_FAIL;

    esp_http_client_set_header(client, "Content-Type", "image/jpeg");

    esp_http_client_set_post_field(
        client,
        (const char*)image_data,
        image_size
    );

    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        ESP_LOGI(
            "HTTP_POST",
            "Imagem enviada. Status: %d",
            esp_http_client_get_status_code(client)
        );
    }

    esp_http_client_cleanup(client);

    return err;
}

esp_err_t GET_command(const char* ip, const char* path, char* response_buffer, size_t buffer_len) {
    char url[128];
    snprintf(url, sizeof(url), "http://%s%s", ip, path);

    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_GET,
        .timeout_ms = 2000,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) return ESP_FAIL;

    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        int read_len = esp_http_client_read_response(client, response_buffer, buffer_len);
        if (read_len >= 0) {
            response_buffer[read_len] = '\0';
        }
    }

    esp_http_client_cleanup(client);
    return err;
}