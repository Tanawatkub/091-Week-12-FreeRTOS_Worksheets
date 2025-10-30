#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_random.h"

static const char *TAG = "COUNTING_SEM_CHALLENGE";

// LEDs
#define LED_R1 GPIO_NUM_2
#define LED_R2 GPIO_NUM_4
#define LED_R3 GPIO_NUM_5
#define LED_SYSTEM GPIO_NUM_18

#define MAX_RESOURCES 3
#define MAX_PRODUCERS 6
#define MIN_RESOURCES 2

typedef struct {
    int id;
    bool in_use;
    bool failed;
    char owner[20];
    uint32_t usage_count;
    uint32_t total_time;
} resource_t;

typedef struct {
    uint32_t req_total;
    uint32_t req_success;
    uint32_t req_fail;
    uint32_t res_failed;
    float utilization;
} stats_t;

resource_t pool[MAX_RESOURCES];
SemaphoreHandle_t xCountingSem;
QueueHandle_t tokenQueue;
stats_t stats = {0};

void blink_led(gpio_num_t pin, int count, int delay) {
    for (int i = 0; i < count; i++) {
        gpio_set_level(pin, 1);
        vTaskDelay(pdMS_TO_TICKS(delay));
        gpio_set_level(pin, 0);
        vTaskDelay(pdMS_TO_TICKS(delay));
    }
}

// Acquire available resource
int acquire_resource(const char *user) {
    for (int i = 0; i < MAX_RESOURCES; i++) {
        if (!pool[i].in_use && !pool[i].failed) {
            pool[i].in_use = true;
            strcpy(pool[i].owner, user);
            pool[i].usage_count++;
            switch (i) {
                case 0: gpio_set_level(LED_R1, 1); break;
                case 1: gpio_set_level(LED_R2, 1); break;
                case 2: gpio_set_level(LED_R3, 1); break;
            }
            return i;
        }
    }
    return -1;
}

// Release resource
void release_resource(int idx, uint32_t time) {
    if (idx >= 0 && idx < MAX_RESOURCES) {
        pool[idx].in_use = false;
        pool[idx].total_time += time;
        strcpy(pool[idx].owner, "");
        switch (idx) {
            case 0: gpio_set_level(LED_R1, 0); break;
            case 1: gpio_set_level(LED_R2, 0); break;
            case 2: gpio_set_level(LED_R3, 0); break;
        }
    }
}

void producer_task(void *pv) {
    int id = *((int*)pv);
    char name[16];
    snprintf(name, sizeof(name), "Producer%d", id);
    int priority = (id % 2 == 0) ? 2 : 1; // even-numbered producers = higher priority
    ESP_LOGI(TAG, "%s started (priority %d)", name, priority);

    while (1) {
        stats.req_total++;
        if (xSemaphoreTake(xCountingSem, pdMS_TO_TICKS(5000)) == pdTRUE) {
            int token;
            if (xQueueReceive(tokenQueue, &token, 0) == pdTRUE) {
                int idx = acquire_resource(name);
                if (idx >= 0) {
                    ESP_LOGI(TAG, "✓ %s: acquired resource %d", name, idx + 1);
                    uint32_t usage = 500 + (esp_random() % 2000);
                    vTaskDelay(pdMS_TO_TICKS(usage));
                    release_resource(idx, usage);
                    xSemaphoreGive(xCountingSem);
                    xQueueSend(tokenQueue, &token, 0);
                    stats.req_success++;
                } else {
                    ESP_LOGW(TAG, "✗ %s: no resource available", name);
                    stats.req_fail++;
                    xSemaphoreGive(xCountingSem);
                }
            }
        } else {
            ESP_LOGW(TAG, "⏰ %s: timeout waiting for semaphore", name);
            stats.req_fail++;
        }

        vTaskDelay(pdMS_TO_TICKS(2000 + (esp_random() % 2000)));
    }
}

// Dynamic scaling
void scaling_task(void *pv) {
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(15000));
        float load = (float)stats.req_success / (stats.req_total + 1);
        if (load < 0.75 && uxSemaphoreGetCount(xCountingSem) < MAX_RESOURCES) {
            xSemaphoreGive(xCountingSem);
            ESP_LOGI(TAG, "⚙️ Load low, increasing available resource slot");
        } else if (load > 0.95 && uxSemaphoreGetCount(xCountingSem) > MIN_RESOURCES) {
            xSemaphoreTake(xCountingSem, 0);
            ESP_LOGW(TAG, "⚠️ Load high, temporarily reducing resource slot");
        }
    }
}

// Failure simulation
void failure_task(void *pv) {
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(25000));
        int idx = esp_random() % MAX_RESOURCES;
        if (!pool[idx].failed) {
            pool[idx].failed = true;
            stats.res_failed++;
            ESP_LOGE(TAG, "💥 Resource %d FAILED!", idx + 1);
            blink_led(LED_SYSTEM, 3, 150);
            vTaskDelay(pdMS_TO_TICKS(5000));
            pool[idx].failed = false;
            ESP_LOGI(TAG, "🧩 Resource %d recovered.", idx + 1);
        }
    }
}

// Monitor system
void monitor_task(void *pv) {
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));
        int available = uxSemaphoreGetCount(xCountingSem);
        int used = MAX_RESOURCES - available;
        stats.utilization = ((float)used / MAX_RESOURCES) * 100;
        ESP_LOGI(TAG, "\n📊 SYSTEM STATUS");
        ESP_LOGI(TAG, "Requests: %lu | Success: %lu | Fail: %lu", stats.req_total, stats.req_success, stats.req_fail);
        ESP_LOGI(TAG, "Resources failed: %lu", stats.res_failed);
        ESP_LOGI(TAG, "Utilization: %.1f%%", stats.utilization);
        for (int i = 0; i < MAX_RESOURCES; i++) {
            ESP_LOGI(TAG, "  Resource %d: %s%s", i + 1,
                     pool[i].failed ? "FAILED " : (pool[i].in_use ? "IN USE" : "FREE"),
                     pool[i].in_use ? pool[i].owner : "");
        }
        ESP_LOGI(TAG, "══════════════════════════════");
    }
}

void app_main(void) {
    gpio_set_direction(LED_R1, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_R2, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_R3, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_SYSTEM, GPIO_MODE_OUTPUT);

    xCountingSem = xSemaphoreCreateCounting(MAX_RESOURCES, MAX_RESOURCES);
    tokenQueue = xQueueCreate(MAX_PRODUCERS, sizeof(int));
    for (int i = 0; i < MAX_PRODUCERS; i++) {
        xQueueSend(tokenQueue, &i, 0);
    }

    if (!xCountingSem || !tokenQueue) {
        ESP_LOGE(TAG, "Initialization failed!");
        return;
    }

    for (int i = 0; i < MAX_RESOURCES; i++) {
        pool[i].id = i + 1;
        pool[i].in_use = false;
        pool[i].failed = false;
    }

    static int ids[MAX_PRODUCERS] = {1, 2, 3, 4, 5, 6};
    for (int i = 0; i < MAX_PRODUCERS; i++) {
        char tname[16];
        snprintf(tname, sizeof(tname), "Producer%d", i + 1);
        xTaskCreate(producer_task, tname, 3072, &ids[i], (i % 2 == 0) ? 4 : 3, NULL);
    }

    xTaskCreate(monitor_task, "Monitor", 3072, NULL, 2, NULL);
    xTaskCreate(scaling_task, "Scaling", 2048, NULL, 2, NULL);
    xTaskCreate(failure_task, "Failure", 2048, NULL, 2, NULL);

    ESP_LOGI(TAG, "💡 Counting Semaphore Challenge Started!");
}
