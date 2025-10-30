#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

static const char *TAG = "QUEUE_EX3";

QueueHandle_t high_priority_queue;
QueueHandle_t normal_priority_queue;

typedef struct {
    char msg[32];
    int priority;
} message_t;

void producer_high_task(void *pv)
{
    message_t msg;
    msg.priority = 1;
    while (1)
    {
        sprintf(msg.msg, "High Priority Msg");
        xQueueSend(high_priority_queue, &msg, pdMS_TO_TICKS(100));
        ESP_LOGW(TAG, "Sent HIGH: %s", msg.msg);
        vTaskDelay(pdMS_TO_TICKS(1500));
    }
}

void producer_normal_task(void *pv)
{
    message_t msg;
    msg.priority = 0;
    int counter = 0;
    while (1)
    {
        sprintf(msg.msg, "Normal Msg %d", counter++);
        xQueueSend(normal_priority_queue, &msg, pdMS_TO_TICKS(100));
        ESP_LOGI(TAG, "Sent NORMAL: %s", msg.msg);
        vTaskDelay(pdMS_TO_TICKS(800));
    }
}

void consumer_task(void *pv)
{
    message_t msg;
    while (1)
    {
        if (xQueueReceive(high_priority_queue, &msg, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            ESP_LOGW(TAG, "⚡ Consumed HIGH: %s", msg.msg);
        }
        else if (xQueueReceive(normal_priority_queue, &msg, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            ESP_LOGI(TAG, "Consumed NORMAL: %s", msg.msg);
        }
        else
        {
            ESP_LOGI(TAG, "No messages, idle...");
        }
        vTaskDelay(pdMS_TO_TICKS(300));
    }
}

void app_main(void)
{
    high_priority_queue = xQueueCreate(5, sizeof(message_t));
    normal_priority_queue = xQueueCreate(15, sizeof(message_t));

    if (!high_priority_queue || !normal_priority_queue)
    {
        ESP_LOGE(TAG, "Failed to create queues!");
        return;
    }

    xTaskCreate(producer_high_task, "HighProducer", 2048, NULL, 3, NULL);
    xTaskCreate(producer_normal_task, "NormalProducer", 2048, NULL, 2, NULL);
    xTaskCreate(consumer_task, "Consumer", 3072, NULL, 4, NULL);
}
