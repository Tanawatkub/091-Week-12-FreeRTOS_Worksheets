#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "TASK_PRIORITY_EX1";

void taskA(void *pvParameters)
{
    while (1)
    {
        ESP_LOGI(TAG, "Task A (Priority 5)");
        vTaskDelay(pdMS_TO_TICKS(1000)); // 1 second
    }
}

void taskB(void *pvParameters)
{
    while (1)
    {
        ESP_LOGI(TAG, "Task B (Priority 3)");
        vTaskDelay(pdMS_TO_TICKS(2000)); // 2 seconds
    }
}

void taskC(void *pvParameters)
{
    while (1)
    {
        ESP_LOGI(TAG, "Task C (Priority 1)");
        vTaskDelay(pdMS_TO_TICKS(3000)); // 3 seconds
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Starting Exercise 1 - Multi-Task System");

    xTaskCreate(taskA, "TaskA", 2048, NULL, 5, NULL);
    xTaskCreate(taskB, "TaskB", 2048, NULL, 3, NULL);
    xTaskCreate(taskC, "TaskC", 2048, NULL, 1, NULL);
}
