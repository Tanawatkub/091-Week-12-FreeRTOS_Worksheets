#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"
#include "esp_task_wdt.h"
#include "driver/gpio.h"
#include "driver/gptimer.h"

static const char *TAG = "ESP32_ADVANCED";

// ============================= UTILITIES =============================
void print_system_info(const char *msg) {
    ESP_LOGI(TAG, "---- %s ----", msg);
    ESP_LOGI(TAG, "Core: %d", xPortGetCoreID());
    ESP_LOGI(TAG, "Free heap: %d bytes", esp_get_free_heap_size());
}

// ============================= EXERCISE 1 =============================
// Dual-Core Task Distribution
void compute_task(void *p) {
    int core = xPortGetCoreID();
    while (1) {
        ESP_LOGI(TAG, "Compute task running on Core %d", core);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void io_task(void *p) {
    int core = xPortGetCoreID();
    while (1) {
        ESP_LOGI(TAG, "I/O task running on Core %d", core);
        vTaskDelay(pdMS_TO_TICKS(1500));
    }
}

void exercise1(void) {
    ESP_LOGI(TAG, "===== Exercise 1: Dual-Core Task Distribution =====");
    xTaskCreatePinnedToCore(compute_task, "ComputeCore0", 2048, NULL, 10, NULL, 0);
    xTaskCreatePinnedToCore(io_task, "IOCore1", 2048, NULL, 8, NULL, 1);
    print_system_info("Tasks distributed across both cores");
}

// ============================= EXERCISE 2 =============================
// Core-Pinned Real-Time System
void realtime_task(void *p) {
    const TickType_t freq = pdMS_TO_TICKS(1);
    TickType_t last = xTaskGetTickCount();
    while (1) {
        // precise 1ms loop
        ESP_LOGI(TAG, "Realtime tick @ %d ms (Core %d)", xTaskGetTickCount() * portTICK_PERIOD_MS, xPortGetCoreID());
        vTaskDelayUntil(&last, freq);
    }
}

void comm_task(void *p) {
    while (1) {
        ESP_LOGI(TAG, "Communication active (Core %d)", xPortGetCoreID());
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void exercise2(void) {
    ESP_LOGI(TAG, "===== Exercise 2: Core-Pinned Real-Time System =====");
    xTaskCreatePinnedToCore(realtime_task, "Realtime", 2048, NULL, 20, NULL, 0);  // Core 0
    xTaskCreatePinnedToCore(comm_task, "Comm", 4096, NULL, 10, NULL, 1);         // Core 1
    print_system_info("Pinned real-time tasks created");
}

// ============================= EXERCISE 3 =============================
// Peripheral Integration (Timer + GPIO)
#define LED_GPIO GPIO_NUM_2

SemaphoreHandle_t timer_sem;

bool IRAM_ATTR timer_callback(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data) {
    BaseType_t hpTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(timer_sem, &hpTaskWoken);
    return hpTaskWoken == pdTRUE;
}

void led_task(void *p) {
    bool state = false;
    while (1) {
        if (xSemaphoreTake(timer_sem, portMAX_DELAY)) {
            state = !state;
            gpio_set_level(LED_GPIO, state);
            ESP_LOGI(TAG, "LED %s (Core %d)", state ? "ON" : "OFF", xPortGetCoreID());
        }
    }
}

void exercise3(void) {
    ESP_LOGI(TAG, "===== Exercise 3: Peripheral Integration =====");
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    timer_sem = xSemaphoreCreateBinary();
    gptimer_handle_t gptimer;
    gptimer_config_t cfg = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000
    };
    gptimer_new_timer(&cfg, &gptimer);

    gptimer_alarm_config_t alarm = {
        .reload_count = 0,
        .alarm_count = 1000000, // 1 second
        .flags.auto_reload_on_alarm = true,
    };
    gptimer_set_alarm_action(gptimer, &alarm);

    gptimer_event_callbacks_t cb = {.on_alarm = timer_callback};
    gptimer_register_event_callbacks(gptimer, &cb, timer_sem);
    gptimer_enable(gptimer);
    gptimer_start(gptimer);

    xTaskCreatePinnedToCore(led_task, "LEDTask", 2048, NULL, 10, NULL, 0);
    print_system_info("Hardware timer active");
}

// ============================= EXERCISE 4 =============================
// Performance Optimization and Monitoring
void benchmark_task(void *p) {
    int core = (int)p;
    int iterations = 0;
    int64_t start = esp_timer_get_time();
    while ((esp_timer_get_time() - start) < 3000000) { // 3 sec
        volatile float res = 0;
        for (int i = 0; i < 1000; i++) res += sqrtf(i * 3.14f);
        iterations++;
    }
    int64_t end = esp_timer_get_time();
    float perf = (float)iterations * 1000000.0f / (end - start);
    ESP_LOGI(TAG, "Core %d: %.2f iterations/sec", core, perf);
    vTaskDelete(NULL);
}

void monitor_task(void *p) {
    while (1) {
        ESP_LOGI(TAG, "System Monitor:");
        ESP_LOGI(TAG, "Free Heap: %d", esp_get_free_heap_size());
        ESP_LOGI(TAG, "Min Heap: %d", esp_get_minimum_free_heap_size());
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void exercise4(void) {
    ESP_LOGI(TAG, "===== Exercise 4: Performance Monitoring =====");
    xTaskCreatePinnedToCore(benchmark_task, "Bench0", 2048, (void*)0, 10, NULL, 0);
    xTaskCreatePinnedToCore(benchmark_task, "Bench1", 2048, (void*)1, 10, NULL, 1);
    xTaskCreate(monitor_task, "Monitor", 3072, NULL, 5, NULL);
    print_system_info("Performance monitoring active");
}

// ============================= MAIN =============================
void app_main(void) {
    ESP_LOGI(TAG, "===== ESP32 FreeRTOS Advanced Exercises =====");
    print_system_info("System Boot");

    int mode = 4; // 🔧 1–4: เปลี่ยนโหมดได้ตามต้องการ

    switch (mode) {
        case 1: exercise1(); break;
        case 2: exercise2(); break;
        case 3: exercise3(); break;
        case 4: exercise4(); break;
        default: ESP_LOGW(TAG, "Invalid mode");
    }
}
