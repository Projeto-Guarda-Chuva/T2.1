#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "nvs_flash.h"

#define MAX_RETRY 5

static const char *TAG = "WIFI_LIB";
static int s_retry_num = 0;

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < MAX_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "Tentando reconectar ao AP...");
        } else {
            ESP_LOGE(TAG, "Falha ao conectar após %d tentativas", MAX_RETRY);
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Conectado! IP: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
    }
}

void wifi_init(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, NULL));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
}


void set_static_ip(esp_netif_t *netif, const char *ip, const char *gateway, const char *netmask) {
    esp_netif_ip_info_t ip_info;
    
    if (esp_netif_str_to_ip4(ip, &ip_info.ip) != ESP_OK ||
        esp_netif_str_to_ip4(gateway, &ip_info.gw) != ESP_OK ||
        esp_netif_str_to_ip4(netmask, &ip_info.netmask) != ESP_OK) {
        ESP_LOGE(TAG, "Erro na conversão dos IPs estáticos");
        return;
    }

    // parando o DHCP para configurar o IP estático
    if (esp_netif_get_handle_from_ifkey("WIFI_STA_DEF") == netif)
        esp_netif_dhcpc_stop(netif);
    else
        esp_netif_dhcps_stop(netif);

    ESP_ERROR_CHECK(esp_netif_set_ip_info(netif, &ip_info));
    ESP_LOGI(TAG, "Configuração de IP estático aplicada: %s", ip);
}

void wifi_start_ap() {
    esp_netif_t *ap_netif = esp_netif_create_default_wifi_ap();

    // esp_netif_dhcps_stop(ap_netif);

    wifi_config_t wifi_config = {0};
    strcpy((char *)wifi_config.ap.ssid, "ESP32-P4");
    strcpy((char *)wifi_config.ap.ssid, "ESP32-P4");

    wifi_config.ap.ssid_len = strlen("ESP32-P4");
    wifi_config.ap.channel = 1;
    wifi_config.ap.max_connection = 4;
    wifi_config.ap.authmode = WIFI_AUTH_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "AP configurado em 192.168.4.1");
}


void wifi_start_sta(const char *my_ip) {
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();

    // set_static_ip(sta_netif, my_ip, "192.168.4.1", "255.255.255.0");

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "ESP32-C6",
            .password = "12345678",
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    
    esp_wifi_connect();
}