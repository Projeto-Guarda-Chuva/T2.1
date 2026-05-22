#include <stdio.h>
#include "wifi.h"
#include "http_comm.h"

static const char* TAG = "CAMERA";

void app_main(void) {
    wifi_init();
    wifi_start_sta("192.168.4.2");

    vTaskDelay(pdMS_TO_TICKS(2000));

    POST_command("192.168.4.1", "/teste", "Hello World!");
}
