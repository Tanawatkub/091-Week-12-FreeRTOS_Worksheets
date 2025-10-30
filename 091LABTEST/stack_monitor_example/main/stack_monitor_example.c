#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "STACK_MONITOR_EX3";

void compute_task(void *pvParameters)
{
    while (1)
    {
        int sum = 0;
        for (int i = 0; i < 10000; i++)
            sum += i;

        ESP_LOGI(TAG, "Computation done, sum = %d", sum);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void stack_monitor_task(void *pvParameters)
{
    while (1)
    {
        UBaseType_t stack_remain = uxTaskGetStackHighWaterMark(NULL);
        ESP_LOGI(TAG, "Stack remaining: %d bytes", stack_remain * sizeof(StackType_t));

        if (stack_remain < 200)
        {
            ESP_LOGW(TAG, "⚠️ Stack usage exceeds 80%% threshold!");
        }

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Starting Exercise 3 - Stack Monitoring");

    xTaskCreate(compute_task, "Compute", 4096, NULL, 2, NULL);
    xTaskCreate(stack_monitor_task, "StackMonitor", 4096, NULL, 1, NULL);
}
