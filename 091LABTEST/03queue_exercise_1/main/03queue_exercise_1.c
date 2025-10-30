#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

static const char *TAG = "QUEUE_EX1";

QueueHandle_t number_queue;

// Producer Task
void producer_task(void *parameter)
{
    int value;

    while (1)
    {
        value = rand() % 100; // Random 0–99
        if (xQueueSend(number_queue, &value, pdMS_TO_TICKS(1000)) == pdTRUE)
        {
            ESP_LOGI(TAG, "Produced: %d", value);
        }
        else
        {
            ESP_LOGW(TAG, "Queue full, producer waiting...");
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

// Consumer Task
void consumer_task(void *parameter)
{
    int received_value;
    float sum = 0;
    int count = 0;

    while (1)
    {
        if (xQueueReceive(number_queue, &received_value, portMAX_DELAY) == pdTRUE)
        {
            count++;
            sum += received_value;
            float avg = sum / count;

            ESP_LOGI(TAG, "Consumed: %d | Average: %.2f", received_value, avg);
        }
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Starting Basic Producer–Consumer Example");

    // Create Queue with 10 integer slots
    number_queue = xQueueCreate(10, sizeof(int));

    if (number_queue == NULL)
    {
        ESP_LOGE(TAG, "Failed to create queue!");
        return;
    }

    // Create Tasks
    xTaskCreate(producer_task, "Producer", 2048, NULL, 2, NULL);
    xTaskCreate(consumer_task, "Consumer", 2048, NULL, 2, NULL);
}
