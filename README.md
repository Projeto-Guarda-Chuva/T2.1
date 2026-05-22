# Guia de Utilização da Biblioteca

Esse é um guia de utilização da biblioteca.

## Contextualização

Esse componente provê a comunicação entre as placas. A ESP32-P4 provê a rede Wi-Fi, atuando como Acess Point (AP). As outras duas placas, ESP32-S3 e ESP8266 R1 D2 Wemos, atuam em modo Station (STA).

## Sobre os components disponíveis

Atualmente, a biblioteca possui dois components principais:
- `wifi_manager`
- `http_comm`

### Component wifi_manager

Responsável pela inicialização e configuração da rede Wi‑Fi das placas. A ESP32-P4 utiliza o modo AP. As demais placas utilizam o modo STA.

### Sobre as funções disponíveis

```C
esp_err_t wifi_init(void);
```

Inicializa a stack de Wi‑Fi da ESP. Essa função deve ser chamada antes de qualquer configuração de rede.

```C
esp_err_t wifi_start_ap(void);
```

Configura a placa como Access Point. Utilizado principalmente na ESP32-P4.

```C
esp_err_t wifi_connect_sta(const char *ssid, const char *password);
```

Conecta a placa em uma rede Wi‑Fi existente. Utilizado pelas placas que operam como STA.


### Component http_comm

Responsável pela comunicação HTTP entre as placas. Cada dispositivo pode:
- Criar endpoints HTTP;
- Receber requisições;
- Enviar requisições para outros dispositivos.

```C
esp_err_t http_server_start(void);
```
Inicia um servidor HTTP. Existe um único servidor por placa.

```C
esp_err_t register_POST_endpoint_queue(const char* uri, QueueHandle_t queue);
```
Registra um endpoint para requisições que utilizem o método POST. Como parâmetros, temos o caminho (ex: /imagens) e uma fila, onde os dados recebidos serão disponibilizados e poderão ser acessados pela task que a criou. Cada endpoint deve possuir uma fila dedicada. 

```C
esp_err_t register_GET_endpoint_callback(const char* uri, http_get_data_provider_t func);
```
Registra um endpoint para requisições que utilizem o método GET. Também temos o caminho (ex: /status), mas possui, em vez de uma fila, uma função de callback como parâmetro. Quando um GET ocorre para o endpoint específicado, a função de callback associada a ele é executada, retornando os dados que serão inseridos na resposta. 

```C
esp_err_t POST_command(const char* ip, const char* path, const char* message);
```
Função para realizar uma requisição de método POST. Como parâmetros, demanda o IP da placa que se quer alcançar e o endpoint registrado, além da própria mensagem. 

```C
esp_err_t GET_command(const char* ip, const char* path, char* response_buffer, size_t buffer_len);
```
Função para realizar uma requisição de método GET. Como parâmetros, demanda o IP da placa que se quer alcançar, o endpoint registrado, um ponteiro do buffer para a resposta e o tamanho do buffer. O parâmetro `buffer_len` é utilizado para evitar escrita fora dos limites do buffer de resposta. O tamanho do buffer deve ser suficiente para comportar toda a resposta recebida. Caso a resposta exceda o tamanho disponível, os dados poderão ser truncados.

## Exemplo - ESP32-P4 (Access Point)

Esse é um exemplo de código para a função da placa em questão. 

```C
void app_main(void) {
    wifi_init();
    wifi_start_ap();

    vTaskDelay(pdMS_TO_TICKS(2000));

    http_server_start();    

    print_task_init();
    monitor_init();
}
```

Nesse exemplo, algumas funções obrigatórias para que a comunicação possa ocorrer:

- Primeiramente, deve-se inicializar o Wi-Fi da placa;
- Em seguida, a placa é configurada como AP;
- Além disso, um servidor HTTP é inicializado para que a placa consiga receber requisições;
- Por fim, são iniciadas uma task básica de impressão e o monitoramento. Ambas serão explicadas adiante. 

## Sobre a estrutura geral de uma Task

A implementação de uma Task, geralmente, consistirá em quatro elementos básicos:

- Uma função de inicialização da Task;
- Uma função para o laço da Task;
- Funções de callback para eventuais endpoints registrados no servidor;
- Demais funções auxiliares relativas ao funcionamento interno da Task.

Esse é o padrão que sugerimos que sigam em suas implementações. Os arquivos ``print.c`` e ``monitor.c`` e seus respectivos ``.h`` podem servir como guias de implementação. De qualquer forma, detalhamos melhor esses elementos em seguida.

No que tange a comunicação, as tasks podem:
- Registrar endpoints (POST e GET);
- Receber dados de outros dispositivos;
- Enviar dados para outros dispositivos

Em endpoints POST, os dados recebidos ficam disponíveis através de filas FreeRTOS.

#### Exemplo - Criando Endpoint POST

```C
QueueHandle_t command_queue;

command_queue = xQueueCreate(5, sizeof(char*));

register_POST_endpoint_queue("/command", command_queue);
```

#### Exemplo — Recebendo Dados da Fila


```C
char *message;

if(xQueueReceive(command_queue, &message, portMAX_DELAY)) {

    ESP_LOGI(TAG, "Mensagem recebida: %s", message);

    free(message);
}
```

Para endpoints GET, uma função de callback deve ser especificada. Essa função será responsável por prover os dados que serão enviados como resposta para a requisição.

#### Exemplo - Criando endpoint GET

```C
register_GET_endpoint_callback("/status", get_status);
```

#### Exemplo - Callback GET

```C
char* get_status(void) {

    char *response = malloc(128);

    snprintf(response, 128,
        "{\"status\":\"online\",\"heap\":%lu}",
        esp_get_free_heap_size()
    );

    return response;
}
```

Além disso, um exemplo simples das funções para envio de requisições POST e GET.

#### Exemplo - Enviando POST

```C
POST_command(
    "192.168.4.1",
    "/print",
    "Hello World"
);
```

#### Exemplo - Enviando GET

```C
char response[256];

GET_command(
    "192.168.4.1",
    "/status",
    response,
    sizeof(response)
);

ESP_LOGI(TAG, "Resposta: %s", response);
```

## Fluxo básico da comunicação

1. A ESP32-P4 inicia a rede Wi-Fi em modo AP;
2. As demais placas conectam-se utilizando STA;
3. O servidor HTTP é iniciado;
4. Endpoints HTTP são registrados;
5. Dispositivos enviam mensagens utilizando requisições HTTP;
6. Endpoints POST encaminham os dados recebidos para as filas FreeRTOS;
7. Endpoints GET respondem com os dados requisitados;
8. Tasks processam os dados recebidos.

## Organização atual da comunicação

Atualmente, as mensagens são enviadas utilizando HTTP, mas o formato do conteúdo das mensagens pode ser definido conforme sua necessidade. A biblioteca fornece apenas:

- Gerenciamento da rede Wi‑Fi;
- Criação do servidor HTTP;
- Registro de endpoints;
- Envio e recebimento de requisições;
- Integração com filas FreeRTOS.

## Observações

- Cada endpoint POST deve possuir sua própria fila;
- Mensagens recebidas via POST são alocadas dinamicamente e devem ser liberadas utilizando ``free()``;
- O servidor HTTP deve ser iniciado antes do registro dos endpoints.
