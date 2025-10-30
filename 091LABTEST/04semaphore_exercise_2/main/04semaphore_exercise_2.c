#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"

static const char *TAG = "EX2_PROD_CONS";

// -----------------------------
// ⚙️ CONFIGURATION
// -----------------------------
#define BUFFER_SIZE 5
#define PRODUCER_DELAY_MS 700
#define CONSUMER_DELAY_MS 1200

// -----------------------------
// 📦 Shared Circular Buffer
// -----------------------------
typedef struct {
    int buffer[BUFFER_SIZE];
    int write_idx;
    int read_idx;
    int count;
    SemaphoreHandle_t mutex;
    SemaphoreHandle_t not_full;
    SemaphoreHandle_t not_empty;
} circ_buffer_t;

circ_buffer_t cb;

// -----------------------------
// 🧩 Utility Function
// -----------------------------
void print_buffer_state()
{
    printf("Buffer: [");
    for (int i = 0; i < BUFFER_SIZE; i++) {
        if (i < cb.count) printf("■");
        else printf("□");
    }
    printf("]  (%d/%d)\n", cb.count, BUFFER_SIZE);
}

// -----------------------------
// 🏭 Producer Task
// -----------------------------
void producer_task(void *param)
{
    int item = 0;

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(PRODUCER_DELAY_MS));

        // รอจนกว่าจะมีที่ว่างใน buffer
        if (xSemaphoreTake(cb.not_full, portMAX_DELAY) == pdTRUE) {
            // เข้าสู่ Critical Section
            if (xSemaphoreTake(cb.mutex, pdMS_TO_TICKS(500)) == pdTRUE) {
                cb.buffer[cb.write_idx] = item;
                cb.write_idx = (cb.write_idx + 1) % BUFFER_SIZE;
                cb.count++;

                ESP_LOGI(TAG, "🟢 Produced item %d", item);
                print_buffer_state();

                xSemaphoreGive(cb.mutex);
                // แจ้ง consumer ว่ามีของแล้ว
                xSemaphoreGive(cb.not_empty);

                item++;
            }
        }
    }
}

// -----------------------------
// 🍽️ Consumer Task
// -----------------------------
void consumer_task(void *param)
{
    int item = 0;

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(CONSUMER_DELAY_MS));

        // รอจนกว่าจะมีของใน buffer
        if (xSemaphoreTake(cb.not_empty, portMAX_DELAY) == pdTRUE) {
            // เข้าสู่ Critical Section
            if (xSemaphoreTake(cb.mutex, pdMS_TO_TICKS(500)) == pdTRUE) {
                item = cb.buffer[cb.read_idx];
                cb.read_idx = (cb.read_idx + 1) % BUFFER_SIZE;
                cb.count--;

                ESP_LOGI(TAG, "🔴 Consumed item %d", item);
                print_buffer_state();

                xSemaphoreGive(cb.mutex);
                // แจ้ง producer ว่ามีที่ว่างแล้ว
                xSemaphoreGive(cb.not_full);
            }
        }
    }
}

// -----------------------------
// 🚀 app_main()
// -----------------------------
void app_main(void)
{
    ESP_LOGI(TAG, "=== Exercise 2: Producer-Consumer Buffer ===");

    cb.write_idx = 0;
    cb.read_idx = 0;
    cb.count = 0;

    cb.mutex = xSemaphoreCreateMutex();
    cb.not_full = xSemaphoreCreateCounting(BUFFER_SIZE, BUFFER_SIZE); // เริ่มต้นเต็ม
    cb.not_empty = xSemaphoreCreateCounting(BUFFER_SIZE, 0);          // เริ่มต้นว่างเปล่า

    if (!cb.mutex || !cb.not_full || !cb.not_empty) {
        ESP_LOGE(TAG, "Failed to create semaphores!");
        return;
    }

    xTaskCreate(producer_task, "Producer", 2048, NULL, 5, NULL);
    xTaskCreate(consumer_task, "Consumer", 2048, NULL, 5, NULL);

    ESP_LOGI(TAG, "System started successfully!");
}
