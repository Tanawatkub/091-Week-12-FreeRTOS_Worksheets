#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_random.h"
#include "esp_timer.h"

static const char *TAG = "QUEUESETS_ADV";

// LED indicators
#define LED_SENSOR GPIO_NUM_2
#define LED_USER GPIO_NUM_4
#define LED_NETWORK GPIO_NUM_5
#define LED_PROCESSOR GPIO_NUM_18
#define LED_PROCESSOR2 GPIO_NUM_19

// Handles
QueueHandle_t qSensor, qUser, qNetwork;
QueueSetHandle_t qSet;
SemaphoreHandle_t printMutex;

// -----------------------------
// 🧠 Structures
// -----------------------------
typedef struct {
    char type[10];
    int priority;
    uint64_t created_time;
    char message[100];
} event_t;

typedef struct {
    uint32_t sensor_count, user_count, network_count;
    uint64_t total_latency_us;
    uint32_t total_events;
} stats_t;

stats_t stats = {0};

// -----------------------------
// 🧩 Utility Functions
// -----------------------------
void safe_log(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    if (xSemaphoreTake(printMutex, pdMS_TO_TICKS(200)) == pdTRUE) {
        vprintf(fmt, args);
        xSemaphoreGive(printMutex);
    }
    va_end(args);
}

// -----------------------------
// 🎛️ Producer Tasks
// -----------------------------
void sensor_task(void *pv) {
    event_t e;
    ESP_LOGI(TAG, "Sensor task started");
    while (1) {
        e.created_time = esp_timer_get_time();
        e.priority = 2;
        strcpy(e.type, "Sensor");
        sprintf(e.message, "Temp: %.1f C", 25 + (esp_random() % 150) / 10.0);
        if (xQueueSend(qSensor, &e, 0) == pdPASS)
            safe_log("📊 Sensor queued: %s\n", e.message);
        gpio_set_level(LED_SENSOR, 1); vTaskDelay(pdMS_TO_TICKS(50));
        gpio_set_level(LED_SENSOR, 0);
        vTaskDelay(pdMS_TO_TICKS(2000 + (esp_random() % 2000)));
    }
}

void user_task(void *pv) {
    event_t e;
    ESP_LOGI(TAG, "User task started");
    while (1) {
        e.created_time = esp_timer_get_time();
        e.priority = 3;
        strcpy(e.type, "User");
        sprintf(e.message, "Button %d pressed", 1 + (esp_random() % 3));
        if (xQueueSend(qUser, &e, 0) == pdPASS)
            safe_log("🔘 User queued: %s\n", e.message);
        gpio_set_level(LED_USER, 1); vTaskDelay(pdMS_TO_TICKS(100));
        gpio_set_level(LED_USER, 0);
        vTaskDelay(pdMS_TO_TICKS(4000 + (esp_random() % 2000)));
    }
}

void network_task(void *pv) {
    event_t e;
    ESP_LOGI(TAG, "Network task started");
    const char *msgs[] = {"Alert", "Sync", "Update", "Notify", "Critical"};
    while (1) {
        e.created_time = esp_timer_get_time();
        e.priority = 2 + (esp_random() % 4);
        strcpy(e.type, "Network");
        sprintf(e.message, "%s message (P:%d)", msgs[esp_random() % 5], e.priority);
        if (xQueueSend(qNetwork, &e, 0) == pdPASS)
            safe_log("🌐 Network queued: %s\n", e.message);
        gpio_set_level(LED_NETWORK, 1); vTaskDelay(pdMS_TO_TICKS(50));
        gpio_set_level(LED_NETWORK, 0);
        vTaskDelay(pdMS_TO_TICKS(1000 + (esp_random() % 2000)));
    }
}

// -----------------------------
// ⚖️ Load Balancer: Processor 2
// -----------------------------
void processor2_task(void *pv) {
    event_t e;
    safe_log("⚙️ Processor 2 ready\n");
    while (1) {
        if (xQueueReceive(qNetwork, &e, pdMS_TO_TICKS(1000)) == pdPASS) {
            uint64_t now = esp_timer_get_time();
            uint64_t latency = now - e.created_time;
            stats.total_latency_us += latency;
            stats.total_events++;
            safe_log("🧠 [CPU2] handled Network: %s (lat: %.2fms)\n",
                     e.message, latency / 1000.0);
            stats.network_count++;
            gpio_set_level(LED_PROCESSOR2, 1);
            vTaskDelay(pdMS_TO_TICKS(150));
            gpio_set_level(LED_PROCESSOR2, 0);
        }
    }
}

// -----------------------------
// 🧩 Processor Task (Main)
// -----------------------------
void processor_task(void *pv) {
    QueueSetMemberHandle_t active;
    event_t e;
    ESP_LOGI(TAG, "Processor main started");

    while (1) {
        active = xQueueSelectFromSet(qSet, portMAX_DELAY);
        gpio_set_level(LED_PROCESSOR, 1);

        if (active == qSensor && xQueueReceive(qSensor, &e, 0) == pdPASS) {
            if (e.priority < 2) continue; // Event filtering
            uint64_t now = esp_timer_get_time();
            uint64_t latency = now - e.created_time;
            stats.total_latency_us += latency;
            stats.total_events++;
            safe_log("🔬 [CPU1] Sensor processed: %s (%.2fms)\n", e.message, latency / 1000.0);
            stats.sensor_count++;
        }

        else if (active == qUser && xQueueReceive(qUser, &e, 0) == pdPASS) {
            uint64_t now = esp_timer_get_time();
            uint64_t latency = now - e.created_time;
            stats.total_latency_us += latency;
            stats.total_events++;
            safe_log("🧍 [CPU1] User processed: %s (%.2fms)\n", e.message, latency / 1000.0);
            stats.user_count++;
        }

        else if (active == qNetwork && xQueueReceive(qNetwork, &e, 0) == pdPASS) {
            if (e.priority >= 4) {
                // Hand off high-load jobs to Processor 2
                safe_log("⚡ Redirecting high P(%d) to Processor 2\n", e.priority);
                xQueueSendToBack(qNetwork, &e, 0);
            } else {
                uint64_t now = esp_timer_get_time();
                uint64_t latency = now - e.created_time;
                stats.total_latency_us += latency;
                stats.total_events++;
                safe_log("🌐 [CPU1] Network processed: %s (%.2fms)\n", e.message, latency / 1000.0);
                stats.network_count++;
            }
        }

        gpio_set_level(LED_PROCESSOR, 0);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// -----------------------------
// 🧮 Dynamic Queue Management + Stats
// -----------------------------
void monitor_task(void *pv) {
    ESP_LOGI(TAG, "System monitor running");
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));
        float avg_latency = stats.total_events ? (stats.total_latency_us / 1000.0) / stats.total_events : 0;
        safe_log("\n📊 SYSTEM STATS\n");
        safe_log("Sensor:%lu  User:%lu  Network:%lu  Total:%lu\n",
                 stats.sensor_count, stats.user_count, stats.network_count, stats.total_events);
        safe_log("Average latency: %.2f ms\n", avg_latency);
        safe_log("Queues → Sensor:%d User:%d Network:%d\n",
                 uxQueueMessagesWaiting(qSensor),
                 uxQueueMessagesWaiting(qUser),
                 uxQueueMessagesWaiting(qNetwork));

        // Dynamic queue management demo
        if (stats.total_events > 30 && qNetwork) {
            vQueueDelete(qNetwork);
            safe_log("🧩 Removed Network Queue dynamically!\n");
            qNetwork = NULL;
        }
    }
}

// -----------------------------
// MAIN
// -----------------------------
void app_main(void) {
    ESP_LOGI(TAG, "Starting Advanced Queue Set System...");

    gpio_set_direction(LED_SENSOR, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_USER, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_NETWORK, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_PROCESSOR, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_PROCESSOR2, GPIO_MODE_OUTPUT);

    qSensor = xQueueCreate(5, sizeof(event_t));
    qUser = xQueueCreate(3, sizeof(event_t));
    qNetwork = xQueueCreate(8, sizeof(event_t));
    qSet = xQueueCreateSet(5 + 3 + 8);
    printMutex = xSemaphoreCreateMutex();

    xQueueAddToSet(qSensor, qSet);
    xQueueAddToSet(qUser, qSet);
    xQueueAddToSet(qNetwork, qSet);

    xTaskCreate(sensor_task, "Sensor", 4096, NULL, 2, NULL);
    xTaskCreate(user_task, "User", 4096, NULL, 2, NULL);
    xTaskCreate(network_task, "Network", 4096, NULL, 3, NULL);
    xTaskCreate(processor_task, "Processor", 4096, NULL, 4, NULL);
    xTaskCreate(processor2_task, "Processor2", 4096, NULL, 3, NULL);
    xTaskCreate(monitor_task, "Monitor", 4096, NULL, 1, NULL);

    ESP_LOGI(TAG, "All tasks created successfully!");
}
