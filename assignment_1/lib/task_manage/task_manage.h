#ifndef LIB__TASK_MANAGE__TASK_MANAGE__H
#define LIB__TASK_MANAGE__TASK_MANAGE__H

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/task.h"

typedef struct __attribute__((packed)) {
    SemaphoreHandle_t mutex;
    QueueHandle_t queue;
} task_manage_t;

#endif