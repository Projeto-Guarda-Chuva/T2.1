#ifndef WIFI_H
#define WIFI_H

#include "esp_err.h"

#define WIFI_SSID "ESP32-C6"
#define WIFI_PASS "12345678"

/*
Inicializa os recursos globais necessários para o Wi-Fi.
Deve ser chamada apenas uma vez, antes de iniciar o AP ou STA.
Inicializa NVS, Netif e Event Loop.
*/
void wifi_init(void);

/*
Configura e inicia a placa no modo AP.
A placa assumirá o IP fixo 192.168.4.1.
*/
void wifi_start_ap(void);

/*
Configura e inicia a placa no modo STA.
my_ip: String contendo o IP fixo desejado.
Placa da câmera: 192.168.4.2
Placa do motor: 192.168.4.3
*/
void wifi_start_sta(const char *my_ip);

#endif /* WIFI_H */