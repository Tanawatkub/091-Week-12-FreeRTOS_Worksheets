#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include "esp_random.h"

static const char *TAG = "BINARY_SEM_CHALLENGE";

// --------------------------
// ⚙️ CONFIGURATION
// --------------------------
#define LED_PRODUCER GPIO_NUM_2
#define LED_CONSUMER1 GPIO_NUM_4
#define LED_CONSUMER2 GPIO_NUM_5
#define BUTTON_PIN GPIO_NUM_0

#define MAX_TIMEOUT_MS 5000

// --------------------------
// 📊 STATS STRUCTURE
// --------------------------
typedef struct {
    uint32_t events_sent;
    uint32_t events_received;
    uint32_t timeouts;
    uint32_t button_events;
    double avg_response_time;
} system_stats_t;

system_stats_t stats = {0, 0, 0, 0, 0};

// --------------------------
// 🔐 SEMAPHORE
// --------------------------
SemaphoreHandle_t xBinarySemaphore;

// --------------------------
// ⏱️ Utility
// --------------------------
uint64_t get_time_ms() {
    return esp_timer_get_time() / 1000ULL;
}

// --------------------------
// 🔘 Button ISR
// --------------------------
static void IRAM_ATTR button_isr(void *arg) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(xBinarySemaphore, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// --------------------------
// 🧠 Producer Task
// --------------------------
void producer_task(void *pvParameters) {
    ESP_LOGI(TAG, "Producer started");
    uint32_t counter = 0;

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(2000 + (esp_random() % 3000)));
        counter++;
        stats.events_sent++;

        if (xSemaphoreGive(xBinarySemaphore) == pdPASS) {
            ESP_LOGI(TAG, "🔥 Producer: Event #%d signaled", counter);
            gpio_set_level(LED_PRODUCER, 1);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_set_level(LED_PRODUCER, 0);
        } else {
            ESP_LOGW(TAG, "⚠️ Producer: Semaphore full!");
        }
    }
}

// --------------------------
// 🧍 Consumer Task
// --------------------------
void consumer_task(void *param) {
    int id = (int)param;
    int led = (id == 1) ? LED_CONSUMER1 : LED_CONSUMER2;
    char name[16];
    snprintf(name, sizeof(name), "Consumer%d", id);

    ESP_LOGI(TAG, "%s started (Priority=%d)", name, uxTaskPriorityGet(NULL));

    while (1) {
        ESP_LOGI(TAG, "%s waiting for event...", name);
        uint64_t start = get_time_ms();

        if (xSemaphoreTake(xBinarySemaphore, pdMS_TO_TICKS(MAX_TIMEOUT_MS)) == pdTRUE) {
            uint64_t end = get_time_ms();
            double resp = (double)(end - start);

            stats.events_received++;
            stats.avg_response_time =
                ((stats.avg_response_time * (stats.events_received - 1)) + resp) / stats.events_received;

            ESP_LOGI(TAG, "⚡ %s: Received event (response %.1f ms)", name, resp);

            gpio_set_level(led, 1);
            vTaskDelay(pdMS_TO_TICKS(500 + (esp_random() % 1500)));
            gpio_set_level(led, 0);

        } else {
            stats.timeouts++;
            ESP_LOGW(TAG, "⏰ %s: Timeout waiting for event!", name);

            // Recovery mechanism
            if (xBinarySemaphore == NULL) {
                ESP_LOGE(TAG, "%s: Semaphore corrupted! Recreating...", name);
                xBinarySemaphore = xSemaphoreCreateBinary();
                if (xBinarySemaphore != NULL)
                    ESP_LOGI(TAG, "%s: Semaphore recovered ✅", name);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// --------------------------
// 🧩 Monitor Task
// --------------------------
void monitor_task(void *pvParameters) {
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));

        ESP_LOGI(TAG, "\n==== SEMAPHORE SYSTEM MONITOR ====");
        ESP_LOGI(TAG, "Events Sent     : %lu", stats.events_sent);
        ESP_LOGI(TAG, "Events Received : %lu", stats.events_received);
        ESP_LOGI(TAG, "Timeouts        : %lu", stats.timeouts);
        ESP_LOGI(TAG, "Button Events   : %lu", stats.button_events);
        ESP_LOGI(TAG, "Avg Response(ms): %.1f", stats.avg_response_time);
        ESP_LOGI(TAG, "==============================\n");
    }
}

// --------------------------
// 🚀 app_main()
// --------------------------
void app_main(void) {
    ESP_LOGI(TAG, "Binary Semaphore Challenge Starting...");

    // Setup GPIO
    gpio_set_direction(LED_PRODUCER, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_CONSUMER1, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_CONSUMER2, GPIO_MODE_OUTPUT);
    gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_PIN, GPIO_PULLUP_ONLY);
    gpio_set_intr_type(BUTTON_PIN, GPIO_INTR_NEGEDGE);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_PIN, button_isr, NULL);

    // Create semaphore
    xBinarySemaphore = xSemaphoreCreateBinary();
    if (!xBinarySemaphore) {
        ESP_LOGE(TAG, "Failed to create semaphore!");
        return;
    }

    // Create tasks
    xTaskCreate(producer_task, "Producer", 2048, NULL, 5, NULL);
    xTaskCreate(consumer_task, "Consumer1", 2048, (void *)1, 4, NULL);
    xTaskCreate(consumer_task, "Consumer2", 2048, (void *)2, 2, NULL);
    xTaskCreate(monitor_task, "Monitor", 2048, NULL, 1, NULL);

    ESP_LOGI(TAG, "System ready. Press BOOT button to trigger ISR event!");
}
