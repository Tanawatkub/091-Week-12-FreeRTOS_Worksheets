#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

static const char *TAG = "QUEUE_EX2";

typedef enum { MSG_TEXT, MSG_NUMBER, MSG_COMMAND } msg_type_t;

typedef struct {
    msg_type_t type;
    union {
        char text[32];
        int number;
        char command[16];
    } data;
} message_t;

QueueHandle_t message_queue;

void producer_text_task(void *pv)
{
    message_t msg;
    msg.type = MSG_TEXT;
    while (1)
    {
        strcpy(msg.data.text, "Hello Queue!");
        xQueueSend(message_queue, &msg, pdMS_TO_TICKS(100));
        ESP_LOGI(TAG, "Sent TEXT: %s", msg.data.text);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void producer_number_task(void *pv)
{
    message_t msg;
    msg.type = MSG_NUMBER;
    while (1)
    {
        msg.data.number = rand() % 500;
        xQueueSend(message_queue, &msg, pdMS_TO_TICKS(100));
        ESP_LOGI(TAG, "Sent NUMBER: %d", msg.data.number);
        vTaskDelay(pdMS_TO_TICKS(1500));
    }
}

void producer_command_task(void *pv)
{
    message_t msg;
    msg.type = MSG_COMMAND;
    while (1)
    {
        strcpy(msg.data.command, "RESET");
        xQueueSend(message_queue, &msg, pdMS_TO_TICKS(100));
        ESP_LOGI(TAG, "Sent COMMAND: %s", msg.data.command);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void consumer_task(void *pv)
{
    message_t received;
    while (1)
    {
        if (xQueueReceive(message_queue, &received, portMAX_DELAY) == pdTRUE)
        {
            switch (received.type)
            {
                case MSG_TEXT:
                    ESP_LOGI(TAG, "Processing TEXT: %s", received.data.text);
                    break;
                case MSG_NUMBER:
                    ESP_LOGI(TAG, "Processing NUMBER: %d", received.data.number);
                    break;
                case MSG_COMMAND:
                    ESP_LOGI(TAG, "Processing COMMAND: %s", received.data.command);
                    break;
            }
        }
    }
}

void app_main(void)
{
    message_queue = xQueueCreate(10, sizeof(message_t));

    if (!message_queue)
    {
        ESP_LOGE(TAG, "Failed to create queue!");
        return;
    }

    xTaskCreate(producer_text_task, "TextProducer", 2048, NULL, 3, NULL);
    xTaskCreate(producer_number_task, "NumProducer", 2048, NULL, 3, NULL);
    xTaskCreate(producer_command_task, "CmdProducer", 2048, NULL, 3, NULL);
    xTaskCreate(consumer_task, "Consumer", 3072, NULL, 2, NULL);
}
