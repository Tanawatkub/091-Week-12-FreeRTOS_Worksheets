#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "TASK_MANAGER_EX2";

TaskHandle_t worker1_handle;
TaskHandle_t worker2_handle;

void worker1(void *pvParameters)
{
    while (1)
    {
        ESP_LOGI(TAG, "Worker 1 running...");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void worker2(void *pvParameters)
{
    while (1)
    {
        ESP_LOGI(TAG, "Worker 2 running...");
        vTaskDelay(pdMS_TO_TICKS(1500));
    }
}

void manager(void *pvParameters)
{
    while (1)
    {
        ESP_LOGI(TAG, "Manager: Suspending Worker 1 for 5s");
        vTaskSuspend(worker1_handle);
        vTaskDelay(pdMS_TO_TICKS(5000));

        ESP_LOGI(TAG, "Manager: Resuming Worker 1");
        vTaskResume(worker1_handle);

        ESP_LOGI(TAG, "Manager: Suspending Worker 2 for 3s");
        vTaskSuspend(worker2_handle);
        vTaskDelay(pdMS_TO_TICKS(3000));

        ESP_LOGI(TAG, "Manager: Resuming Worker 2");
        vTaskResume(worker2_handle);
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Starting Exercise 2 - Task Suspension and Resumption");

    xTaskCreate(worker1, "Worker1", 2048, NULL, 3, &worker1_handle);
    xTaskCreate(worker2, "Worker2", 2048, NULL, 3, &worker2_handle);
    xTaskCreate(manager, "Manager", 3072, NULL, 4, NULL);
}
