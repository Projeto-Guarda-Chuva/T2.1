#include "monitor.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "cJSON.h"
#include "http_comm.h"

static const char *TAG = "MONITOR_MODULE";

static char* monitor_callback() {
    cJSON *root = cJSON_CreateObject();

    int64_t uptime = esp_timer_get_time() / 1000;
    uint32_t free_heap = esp_get_free_heap_size();
    uint32_t min_free_heap = esp_get_minimum_free_heap_size();
    
    cJSON_AddNumberToObject(root, "free_heap", free_heap);
    cJSON_AddNumberToObject(root, "min_free_heap", min_free_heap);
    cJSON_AddNumberToObject(root, "uptime_ms", (double)uptime);
    cJSON_AddStringToObject(root, "status", "running");

    char *json_string = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    ESP_LOGI(TAG, "%s", json_string);
    return json_string;
}

void monitor_init(void) {
    register_GET_endpoint_callback("/monitor", monitor_callback);
}