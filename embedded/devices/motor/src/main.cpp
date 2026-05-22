#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#define BOT_A D1
#define BOT_B D2
#define FA D5
#define FF D6

const char *ssid = "ESP_F85DED";
//const char *ssid = "ESP32-P4";
// const char *password = "12345678";

ESP8266WebServer server(80);

enum gate_state {
    GATE_STOPPED,
    GATE_OPENING,
    GATE_CLOSING
};
gate_state state = GATE_CLOSING;

void connect_WiFi();
void handle_command();
void handle_status();

void stop_gate(); 
void open_gate();
void close_gate();

void setup() {
    Serial.begin(115200);

    pinMode(BOT_A, OUTPUT);
    pinMode(BOT_B, OUTPUT);
    pinMode(FA, INPUT);
    pinMode(FF, INPUT);

    digitalWrite(BOT_A, LOW);
    digitalWrite(BOT_B, LOW);

    WiFi.mode(WIFI_STA);
    connect_WiFi();

    server.on("/status", HTTP_GET, handle_status);
    server.on("/command", HTTP_POST, handle_command);

    server.onNotFound([]() {
        server.send(404, "text/plain", "Endpoint não encontrado. Use /status ou /command. ");
    });

    server.begin();
    Serial.println("Servidor HTTP iniciado!");
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Sinal WiFi perdido, reconectando...");
        connect_WiFi();
    }

    // condicionais para que o motor seja parado ao atingir FA ou FF
    if (state == GATE_OPENING && digitalRead(FA) == HIGH)
        stop_gate();
    if (state == GATE_CLOSING && digitalRead(FF) == HIGH)
        stop_gate();

    server.handleClient();
}

void handle_command() {
    if (server.hasArg("plain")) {
        String cmd = server.arg("plain");

        Serial.println("Comando recebido em /command: ");
        Serial.println(cmd);

        if (cmd == "A") {
            if (state == GATE_STOPPED && digitalRead(FA) == LOW)
                open_gate();
            else if (state == GATE_CLOSING)
                stop_gate();
            // ignorando o comando caso o portão esteja abrindo

        } else if (cmd == "F") {
            if (state == GATE_STOPPED && digitalRead(FF) == LOW)
                close_gate();
            else if (state == GATE_OPENING)
                stop_gate();
            // ignorando o comando caso o portão esteja fechando

        } else {
            server.send(400, "text/plain", "Comando Inválido: Use A para abrir ou F para fechar.");            
            return;
        }
        server.send(200, "text/plain", "Comando recebido.");
    } else {
        server.send(400, "text/plain", "Corpo vazio: Use A para abrir ou F para fechar.");
    }
}

void handle_status() {
    Serial.println("Requisição recebida em /status");

    String json = "{";
    json += "\"rssi\":" + String(WiFi.RSSI()) + ",";
    json += "\"heap\":" + String(ESP.getFreeHeap()) + ",";
    json += "\"uptime\":" + String(millis());
    json += "}";

    server.send(200, "application/json", json);

    Serial.println(json);
}

void connect_WiFi() {
    if (WiFi.status() == WL_CONNECTED)
        return;

    // WiFi.begin(ssid, password);
    WiFi.begin(ssid);
    Serial.print("Conectando...");

    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }

    Serial.println("\nConectado!");
    Serial.print("IP do Servidor: ");
    Serial.println(WiFi.localIP());
}

void stop_gate() {
    digitalWrite(BOT_A, LOW);
    digitalWrite(BOT_B, LOW);
    state = GATE_STOPPED;

    Serial.print("O portão foi parado.");
}

void open_gate() {
    digitalWrite(BOT_B, LOW);
    digitalWrite(BOT_A, HIGH);
    state = GATE_OPENING;

    Serial.println("O portão está abrindo.");
}

void close_gate() {
    digitalWrite(BOT_A, LOW);
    digitalWrite(BOT_B, HIGH);
    state = GATE_CLOSING;

    Serial.println("O portão está fechando.");
}