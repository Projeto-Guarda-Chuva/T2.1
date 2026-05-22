idf_component_register(
    SRCS "main.c" "print.c" "monitor.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES "wifi_manager" "http_comm" "esp_wifi_remote" "esp_timer" "json"
)