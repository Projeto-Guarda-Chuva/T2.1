#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/freeRTOS.H"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "http_comm.h"
#include "print.h"

static const char *TAG = "PRINT_MODULE";
QueueHandle_t print_queue = NULL;

static void print_loop(void *pv) {
    char *dados = NULL;
    
    while (1) {
        
        if (xQueueReceive(print_queue, &dados, pdMS_TO_TICKS(10))) {
            if (dados != NULL) {
                ESP_LOGI("PRINT_TASK", "%s", dados);
                free(dados); 
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void print_task_init(void) {
    print_queue = xQueueCreate(10, sizeof(char *));
    if (print_queue == NULL) {
        ESP_LOGE(TAG, "Falha ao criar a fila de impressão.");
        return;
    }
    register_POST_endpoint_queue("/print", print_queue);

    xTaskCreate(print_loop, "print_task", 4096, NULL, 5, NULL);
    
    ESP_LOGI(TAG, "Task de impressão inicializada com sucesso");
}