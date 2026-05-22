#include "esp_wifi_remote.h"
#include "wifi.h"
#include "http_comm.h"
#include "print.h"
#include "monitor.h"

static const char* TAG = "PROCESSADORA";

void app_main(void) {
    wifi_init();
    wifi_start_ap();

    vTaskDelay(pdMS_TO_TICKS(2000));

    http_server_start();    

    print_task_init();
    monitor_init();
}
