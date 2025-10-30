#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "driver/gpio.h"

static const char *TAG = "MUTEX_CHALLENGE";

// LEDs
#define LED_HIGH GPIO_NUM_2
#define LED_MED  GPIO_NUM_4
#define LED_LOW  GPIO_NUM_5
#define LED_CRIT GPIO_NUM_18

// Shared resources
SemaphoreHandle_t mutexA;
SemaphoreHandle_t mutexB;
SemaphoreHandle_t recursiveMutex;

// Stats
typedef struct {
    uint32_t access_ok;
    uint32_t deadlocks;
    uint32_t priority_inversions;
    uint32_t recursion_uses;
} stat_t;
stat_t stats = {0};

// Safe critical section
void access_resource_pair(const char *task, SemaphoreHandle_t first, SemaphoreHandle_t second, gpio_num_t led) {
    ESP_LOGI(TAG, "[%s] requesting first mutex...", task);
    if (xSemaphoreTake(first, pdMS_TO_TICKS(2000)) != pdTRUE) {
        ESP_LOGW(TAG, "[%s] timeout on first mutex", task);
        return;
    }
    gpio_set_level(led, 1);
    vTaskDelay(pdMS_TO_TICKS(200));  // simulate work

    ESP_LOGI(TAG, "[%s] requesting second mutex...", task);
    if (xSemaphoreTake(second, pdMS_TO_TICKS(2000)) != pdTRUE) {
        ESP_LOGE(TAG, "[%s] 💀 Deadlock detected!", task);
        stats.deadlocks++;
        xSemaphoreGive(first);
        gpio_set_level(led, 0);
        return;
    }

    ESP_LOGI(TAG, "[%s] working safely in double-locked section", task);
    vTaskDelay(pdMS_TO_TICKS(300));
    stats.access_ok++;

    xSemaphoreGive(second);
    xSemaphoreGive(first);
    gpio_set_level(led, 0);
}

// Recursive test
void recursive_lock_demo(const char *task, int level) {
    if (xSemaphoreTakeRecursive(recursiveMutex, pdMS_TO_TICKS(500)) == pdTRUE) {
        stats.recursion_uses++;
        ESP_LOGI(TAG, "%s recursion depth %d", task, level);
        vTaskDelay(pdMS_TO_TICKS(100));
        if (level < 3) recursive_lock_demo(task, level + 1);
        xSemaphoreGiveRecursive(recursiveMutex);
    }
}

void high_task(void *pv) {
    ESP_LOGI(TAG, "High Priority task started");
    while (1) {
        access_resource_pair("HIGH", mutexA, mutexB, LED_HIGH);
        recursive_lock_demo("HIGH", 1);
        vTaskDelay(pdMS_TO_TICKS(1500));
    }
}

void med_task(void *pv) {
    ESP_LOGI(TAG, "Medium Priority task started");
    while (1) {
        // simulate CPU load
        for (volatile int i = 0; i < 500000; i++);
        vTaskDelay(pdMS_TO_TICKS(800));
    }
}

void low_task(void *pv) {
    ESP_LOGI(TAG, "Low Priority task started");
    while (1) {
        // lock in opposite order to induce potential deadlock
        access_resource_pair("LOW", mutexB, mutexA, LED_LOW);
        vTaskDelay(pdMS_TO_TICKS(1200));
    }
}

void monitor_task(void *pv) {
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));
        ESP_LOGI(TAG, "\n==== MUTEX CHALLENGE STATS ====");
        ESP_LOGI(TAG, "Access OK         : %lu", stats.access_ok);
        ESP_LOGI(TAG, "Deadlocks Detected: %lu", stats.deadlocks);
        ESP_LOGI(TAG, "Recursive Uses    : %lu", stats.recursion_uses);
        ESP_LOGI(TAG, "===============================\n");
    }
}

void app_main(void) {
    gpio_set_direction(LED_HIGH, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_MED, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_LOW, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_CRIT, GPIO_MODE_OUTPUT);

    mutexA = xSemaphoreCreateMutex();
    mutexB = xSemaphoreCreateMutex();
    recursiveMutex = xSemaphoreCreateRecursiveMutex();

    if (!mutexA || !mutexB || !recursiveMutex) {
        ESP_LOGE(TAG, "Mutex creation failed");
        return;
    }

    xTaskCreate(high_task, "High", 3072, NULL, 5, NULL);
    xTaskCreate(med_task, "Medium", 3072, NULL, 4, NULL);
    xTaskCreate(low_task, "Low", 3072, NULL, 3, NULL);
    xTaskCreate(monitor_task, "Monitor", 3072, NULL, 2, NULL);

    ESP_LOGI(TAG, "💡 Challenge system initialized!");
}
