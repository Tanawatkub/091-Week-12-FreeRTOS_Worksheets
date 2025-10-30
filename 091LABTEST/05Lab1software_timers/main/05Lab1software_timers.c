#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_random.h"

static const char *TAG = "TIMER_CHALLENGE";

// GPIO Pins
#define LED_SYNC GPIO_NUM_2
#define LED_ANALYSIS GPIO_NUM_4
#define LED_ERROR GPIO_NUM_5
#define LED_SCHED GPIO_NUM_18

// Timer Handles
TimerHandle_t syncTimer;
TimerHandle_t analysisTimer;
TimerHandle_t schedulerTimer;
TimerHandle_t errorCheckTimer;

// Global Variables
static int sync_count = 0;
static int error_state = 0;
static TickType_t last_sync_time = 0;
static float avg_interval = 0.0;

// ------------------ Timer Callbacks ------------------

void sync_callback(TimerHandle_t xTimer) {
    sync_count++;
    TickType_t now = xTaskGetTickCount();
    TickType_t interval = (now - last_sync_time);
    last_sync_time = now;

    avg_interval = (avg_interval * (sync_count - 1) + interval) / sync_count;

    gpio_set_level(LED_SYNC, sync_count % 2);
    ESP_LOGI(TAG, "🔄 Sync Timer #%d | Interval: %d ticks | Avg: %.2f", sync_count, interval, avg_interval);

    // Randomly inject simulated delay (testing timer precision)
    if (esp_random() % 10 == 0) {
        ESP_LOGW(TAG, "⚠️ Simulating processing delay...");
        vTaskDelay(pdMS_TO_TICKS(200)); // deliberate delay
    }
}

void analysis_callback(TimerHandle_t xTimer) {
    ESP_LOGI(TAG, "📊 Performance Analysis...");
    gpio_set_level(LED_ANALYSIS, 1);

    // Check if sync timer deviates too much
    if (avg_interval > pdMS_TO_TICKS(600)) {
        ESP_LOGW(TAG, "⏱️ Timer drift detected! Avg interval = %.2f", avg_interval);
    } else {
        ESP_LOGI(TAG, "✅ Timer stable (avg interval %.2f ticks)", avg_interval);
    }

    gpio_set_level(LED_ANALYSIS, 0);
}

void scheduler_callback(TimerHandle_t xTimer) {
    int pattern = esp_random() % 3;

    gpio_set_level(LED_SCHED, 1);
    ESP_LOGI(TAG, "🧭 Scheduler Pattern #%d", pattern);

    switch (pattern) {
        case 0:
            ESP_LOGI(TAG, "🟢 Increasing Sync Speed");
            xTimerChangePeriod(syncTimer, pdMS_TO_TICKS(300), 0);
            break;
        case 1:
            ESP_LOGI(TAG, "🟡 Slowing Down Sync");
            xTimerChangePeriod(syncTimer, pdMS_TO_TICKS(700), 0);
            break;
        case 2:
            ESP_LOGI(TAG, "🔵 Restarting Sync Timer");
            xTimerReset(syncTimer, 0);
            break;
    }

    gpio_set_level(LED_SCHED, 0);
}

void error_check_callback(TimerHandle_t xTimer) {
    if (esp_random() % 15 == 0) {
        error_state = 1;
        gpio_set_level(LED_ERROR, 1);
        ESP_LOGE(TAG, "💥 Simulated Timer Error Detected!");

        // Recovery: stop & restart
        ESP_LOGI(TAG, "🔁 Attempting Recovery...");
        xTimerStop(syncTimer, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
        xTimerStart(syncTimer, 0);
        gpio_set_level(LED_ERROR, 0);
        error_state = 0;
    }
}

// ------------------ App Main ------------------

void app_main(void) {
    ESP_LOGI(TAG, "🚀 Advanced FreeRTOS Timer Challenge Starting...");

    // Setup LEDs
    gpio_set_direction(LED_SYNC, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_ANALYSIS, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_ERROR, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_SCHED, GPIO_MODE_OUTPUT);

    gpio_set_level(LED_SYNC, 0);
    gpio_set_level(LED_ANALYSIS, 0);
    gpio_set_level(LED_ERROR, 0);
    gpio_set_level(LED_SCHED, 0);

    // Create Timers
    syncTimer = xTimerCreate("SyncTimer", pdMS_TO_TICKS(500), pdTRUE, NULL, sync_callback);
    analysisTimer = xTimerCreate("AnalysisTimer", pdMS_TO_TICKS(3000), pdTRUE, NULL, analysis_callback);
    schedulerTimer = xTimerCreate("SchedulerTimer", pdMS_TO_TICKS(7000), pdTRUE, NULL, scheduler_callback);
    errorCheckTimer = xTimerCreate("ErrorCheckTimer", pdMS_TO_TICKS(5000), pdTRUE, NULL, error_check_callback);

    if (!syncTimer || !analysisTimer || !schedulerTimer || !errorCheckTimer) {
        ESP_LOGE(TAG, "❌ Failed to create one or more timers!");
        return;
    }

    // Start all timers
    xTimerStart(syncTimer, 0);
    xTimerStart(analysisTimer, 0);
    xTimerStart(schedulerTimer, 0);
    xTimerStart(errorCheckTimer, 0);

    ESP_LOGI(TAG, "✅ All timers started successfully!");
    ESP_LOGI(TAG, "LED Indicators: GPIO2=Sync | GPIO4=Analysis | GPIO5=Error | GPIO18=Scheduler");
}
