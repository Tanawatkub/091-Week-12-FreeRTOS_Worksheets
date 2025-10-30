#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"

static const char *TAG = "EX4_PRIORITY_INHERIT";

// -----------------------------------
// ⚙️ CONFIGURATION
// -----------------------------------
#define TASK_DELAY_SHORT 100
#define TASK_DELAY_LONG  3000

SemaphoreHandle_t resource_mutex;

// -----------------------------------
// 🧠 Utility: Simulate some CPU work
// -----------------------------------
void do_work(const char *task_name, int ms)
{
    ESP_LOGI(TAG, "%s: working for %d ms...", task_name, ms);
    vTaskDelay(pdMS_TO_TICKS(ms));
}

// -----------------------------------
// 🧍 LOW PRIORITY TASK
// -----------------------------------
void low_priority_task(void *param)
{
    const char *name = "LOW";

    while (1) {
        ESP_LOGI(TAG, "%s: trying to acquire mutex...", name);

        if (xSemaphoreTake(resource_mutex, portMAX_DELAY) == pdTRUE) {
            ESP_LOGI(TAG, "%s: acquired mutex ✅", name);

            // ใช้ resource นานมาก
            do_work(name, TASK_DELAY_LONG);

            ESP_LOGI(TAG, "%s: releasing mutex 🔓", name);
            xSemaphoreGive(resource_mutex);
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

// -----------------------------------
// ⚙️ MEDIUM PRIORITY TASK (CPU-Intensive)
// -----------------------------------
void medium_priority_task(void *param)
{
    const char *name = "MEDIUM";

    while (1) {
        ESP_LOGI(TAG, "%s: doing background CPU work...", name);
        // ทำ CPU-intensive (แต่ไม่ใช้ mutex)
        for (volatile int i = 0; i < 500000; i++);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// -----------------------------------
// 🚀 HIGH PRIORITY TASK
// -----------------------------------
void high_priority_task(void *param)
{
    const char *name = "HIGH";

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(5000));  // เริ่มหลังสุด

        ESP_LOGW(TAG, "%s: NEED mutex now! 🔥", name);

        if (xSemaphoreTake(resource_mutex, portMAX_DELAY) == pdTRUE) {
            ESP_LOGI(TAG, "%s: acquired mutex ✅ (after waiting)", name);
            do_work(name, 1000);
            ESP_LOGI(TAG, "%s: releasing mutex 🔓", name);
            xSemaphoreGive(resource_mutex);
        }
    }
}

// -----------------------------------
// 🧩 app_main()
// -----------------------------------
void app_main(void)
{
    ESP_LOGI(TAG, "=== Exercise 4: Priority Inheritance Demo ===");

    // สร้าง Mutex (มี Priority Inheritance โดยค่าเริ่มต้น)
    resource_mutex = xSemaphoreCreateMutex();
    if (resource_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create mutex!");
        return;
    }

    // สร้าง Task 3 ตัว
    // Note: Priority สูง = ตัวเลขมาก
    xTaskCreate(low_priority_task, "LowTask", 3072, NULL, 5, NULL);     // ต่ำ
    xTaskCreate(medium_priority_task, "MedTask", 3072, NULL, 10, NULL); // กลาง
    xTaskCreate(high_priority_task, "HighTask", 3072, NULL, 15, NULL);  // สูง

    ESP_LOGI(TAG, "Tasks created: Low=5, Med=10, High=15 (inheritance ON)");
}
