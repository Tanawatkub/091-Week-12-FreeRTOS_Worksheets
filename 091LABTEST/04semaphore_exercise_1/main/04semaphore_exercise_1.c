#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"

static const char *TAG = "EX1_RESOURCE_PROTECT";

// ----------------------------
// ⚙️ CONFIG
// ----------------------------
#define USE_MUTEX 1   // 0 = ไม่ใช้ (Race Condition), 1 = ใช้ (Protected)

// ----------------------------
// 📊 Shared Resource
// ----------------------------
static int shared_counter = 0;
SemaphoreHandle_t counter_mutex;

// ----------------------------
// 🧵 Task Function
// ----------------------------
void counter_task(void *param)
{
    int id = (int)param;
    char task_name[10];
    sprintf(task_name, "TASK%d", id);

    while (1)
    {
#if USE_MUTEX
        // --- 🔒 เข้าสู่ Critical Section แบบปลอดภัย ---
        if (xSemaphoreTake(counter_mutex, pdMS_TO_TICKS(500)) == pdTRUE)
        {
            int local = shared_counter;
            ESP_LOGI(TAG, "%s: Acquired mutex, read counter = %d", task_name, local);
            vTaskDelay(pdMS_TO_TICKS(200));  // จำลอง delay ก่อนเขียน
            shared_counter = local + 1;
            ESP_LOGI(TAG, "%s: Updated counter to %d", task_name, shared_counter);

            xSemaphoreGive(counter_mutex);
            ESP_LOGI(TAG, "%s: Released mutex\n", task_name);
        }
        else
        {
            ESP_LOGW(TAG, "%s: Timeout acquiring mutex!", task_name);
        }
#else
        // --- ⚠️ ไม่ใช้ mutex → Race Condition ---
        int local = shared_counter;
        ESP_LOGI(TAG, "%s: Read counter: %d, incrementing...", task_name, local);
        vTaskDelay(pdMS_TO_TICKS(200));
        shared_counter = local + 1;
        ESP_LOGI(TAG, "%s: Updated counter to %d\n", task_name, shared_counter);
#endif

        vTaskDelay(pdMS_TO_TICKS(500));  // เวลาพักก่อนวนรอบต่อไป
    }
}

// ----------------------------
// 🚀 app_main
// ----------------------------
void app_main(void)
{
    ESP_LOGI(TAG, "=== Exercise 1: Resource Protection ===");
    ESP_LOGI(TAG, "Mode: %s\n", USE_MUTEX ? "🔒 Using Mutex" : "⚠️ No Mutex (Race Condition)");

    if (USE_MUTEX)
    {
        counter_mutex = xSemaphoreCreateMutex();
        if (counter_mutex == NULL)
        {
            ESP_LOGE(TAG, "Failed to create mutex!");
            return;
        }
    }

    xTaskCreate(counter_task, "Task1", 2048, (void *)1, 5, NULL);
    xTaskCreate(counter_task, "Task2", 2048, (void *)2, 5, NULL);
    xTaskCreate(counter_task, "Task3", 2048, (void *)3, 5, NULL);
}
