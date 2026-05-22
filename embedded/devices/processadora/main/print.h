#ifndef PRINT_H
#define PRINT_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

extern QueueHandle_t print_queue;

void print_task_init(void);

#endif // PRINT_H