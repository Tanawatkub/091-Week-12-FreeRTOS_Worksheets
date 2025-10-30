#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_system.h"
#include "esp_random.h"
#include "esp_sntp.h"
#include "driver/gpio.h"
#include "nvs_flash.h"

static const char *TAG = "EXPERT_CHALLENGES";

// =============================
// CONFIGURATION
// =============================
#define MAX_SCHEDULED_TASKS     10
#define MEMORY_THRESHOLD        30000
#define NETWORK_SYNC_INTERVAL   10000
#define LOAD_BALANCE_INTERVAL   2000

#define LED_SCHEDULER   GPIO_NUM_2
#define LED_SYNC        GPIO_NUM_4
#define LED_LOAD        GPIO_NUM_5
#define LED_ERROR       GPIO_NUM_18

// =============================
// DATA STRUCTURES
// =============================
typedef struct {
    char name[16];
    uint32_t deadline_ms;
    uint8_t priority;
    TimerHandle_t timer;
    TickType_t period;
    uint32_t last_exec_time;
    bool active;
} scheduled_task_t;

typedef struct {
    float avg_exec_us;
    uint32_t task_count;
    float cpu_load;
    float accuracy;
    uint32_t missed_deadlines;
} system_metrics_t;

// =============================
// GLOBAL VARIABLES
// =============================
static scheduled_task_t scheduler_pool[MAX_SCHEDULED_TASKS];
static SemaphoreHandle_t scheduler_mutex;
static system_metrics_t metrics = {0};

// =============================
// PROTOTYPES
// =============================
void scheduler_task_callback(TimerHandle_t timer);
void real_time_scheduler_task(void *param);
void network_sync_task(void *param);
void adaptive_controller_task(void *param);
void check_memory_health(void);
void comprehensive_health_check(void);
BaseType_t safe_timer_start(TimerHandle_t timer);

// =============================
// REAL-TIME SCHEDULER SYSTEM
// =============================
void init_scheduler(void) {
    scheduler_mutex = xSemaphoreCreateMutex();
    for (int i = 0; i < MAX_SCHEDULED_TASKS; i++) {
        scheduler_pool[i].active = false;
    }
    ESP_LOGI(TAG, "Scheduler initialized with %d slots", MAX_SCHEDULED_TASKS);
}

BaseType_t schedule_task(const char *name, uint32_t period_ms, uint8_t priority, uint32_t deadline_ms) {
    if (xSemaphoreTake(scheduler_mutex, pdMS_TO_TICKS(100)) != pdTRUE)
        return pdFAIL;

    for (int i = 0; i < MAX_SCHEDULED_TASKS; i++) {
        if (!scheduler_pool[i].active) {
            snprintf(scheduler_pool[i].name, sizeof(scheduler_pool[i].name), "%s", name);
            scheduler_pool[i].period = pdMS_TO_TICKS(period_ms);
            scheduler_pool[i].priority = priority;
            scheduler_pool[i].deadline_ms = deadline_ms;
            scheduler_pool[i].timer = xTimerCreate(name, scheduler_pool[i].period, pdTRUE, (void*)i, scheduler_task_callback);
            scheduler_pool[i].active = true;

            if (scheduler_pool[i].timer)
                safe_timer_start(scheduler_pool[i].timer);

            xSemaphoreGive(scheduler_mutex);
            ESP_LOGI(TAG, "Scheduled task: %s | Priority=%d | Deadline=%dms", name, priority, deadline_ms);
            return pdPASS;
        }
    }

    xSemaphoreGive(scheduler_mutex);
    return pdFAIL;
}

void scheduler_task_callback(TimerHandle_t timer) {
    int idx = (int)pvTimerGetTimerID(timer);
    uint32_t start = esp_timer_get_time();
    gpio_set_level(LED_SCHEDULER, 1);

    // Simulate computation based on priority
    uint32_t work_time = 100 + (scheduler_pool[idx].priority * 50);
    ets_delay_us(work_time * 10); // simulate load

    uint32_t duration = esp_timer_get_time() - start;
    metrics.avg_exec_us = (metrics.avg_exec_us + duration) / 2.0;
    scheduler_pool[idx].last_exec_time = start / 1000;

    // Check deadline
    if (duration / 1000 > scheduler_pool[idx].deadline_ms) {
        metrics.missed_deadlines++;
        ESP_LOGW(TAG, "⏰ Missed deadline: %s (%luμs > %dms)", scheduler_pool[idx].name, duration, scheduler_pool[idx].deadline_ms);
    }

    gpio_set_level(LED_SCHEDULER, 0);
}

// =============================
// DISTRIBUTED TIMER SYSTEM
// =============================
void network_sync_task(void *param) {
    ESP_LOGI(TAG, "🌐 Distributed Timer Sync started");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();

    while (1) {
        time_t now;
        time(&now);
        ESP_LOGI(TAG, "🕒 Network time sync: %s", ctime(&now));
        gpio_set_level(LED_SYNC, 1);
        vTaskDelay(pdMS_TO_TICKS(200));
        gpio_set_level(LED_SYNC, 0);

        vTaskDelay(pdMS_TO_TICKS(NETWORK_SYNC_INTERVAL));
    }
}

// =============================
// ADAPTIVE PERFORMANCE CONTROLLER
// =============================
void adaptive_controller_task(void *param) {
    ESP_LOGI(TAG, "🧠 Adaptive performance controller running");
    float load_factor = 1.0;

    while (1) {
        check_memory_health();
        comprehensive_health_check();

        // Adjust periods dynamically based on load
        if (metrics.cpu_load > 80.0) load_factor = 1.2;
        else if (metrics.cpu_load < 40.0) load_factor = 0.8;
        else load_factor = 1.0;

        if (xSemaphoreTake(scheduler_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            for (int i = 0; i < MAX_SCHEDULED_TASKS; i++) {
                if (scheduler_pool[i].active) {
                    TickType_t new_period = scheduler_pool[i].period * load_factor;
                    xTimerChangePeriod(scheduler_pool[i].timer, new_period, 0);
                }
            }
            xSemaphoreGive(scheduler_mutex);
        }

        gpio_set_level(LED_LOAD, 1);
        vTaskDelay(pdMS_TO_TICKS(LOAD_BALANCE_INTERVAL / 2));
        gpio_set_level(LED_LOAD, 0);

        vTaskDelay(pdMS_TO_TICKS(LOAD_BALANCE_INTERVAL));
    }
}

// =============================
// PRODUCTION UTILITIES
// =============================
void check_memory_health(void) {
    size_t free_heap = esp_get_free_heap_size();
    if (free_heap < MEMORY_THRESHOLD) {
        ESP_LOGW(TAG, "⚠️ Low memory: %d bytes", free_heap);
        gpio_set_level(LED_ERROR, 1);
    } else {
        gpio_set_level(LED_ERROR, 0);
    }
}

BaseType_t safe_timer_start(TimerHandle_t timer) {
    BaseType_t result = pdFAIL;
    for (int retry = 0; retry < 3; retry++) {
        result = xTimerStart(timer, pdMS_TO_TICKS(100));
        if (result == pdPASS) break;
        vTaskDelay(pdMS_TO_TICKS(10));
        ESP_LOGW(TAG, "Timer start retry %d", retry + 1);
    }
    return result;
}

void comprehensive_health_check(void) {
    TaskStatus_t status;
    vTaskGetInfo(xTimerGetTimerDaemonTaskHandle(), &status, pdTRUE, eInvalid);
    uint32_t total_run_time = esp_timer_get_time() / 1000;
    uint32_t load = (status.ulRunTimeCounter * 100) / (total_run_time + 1);
    metrics.cpu_load = load;

    if (status.usStackHighWaterMark < 100) {
        ESP_LOGW(TAG, "Low stack: %d words left", status.usStackHighWaterMark);
    }
    if (load > 80) {
        ESP_LOGW(TAG, "⚠️ High CPU Load: %lu%%", load);
    }
}

// =============================
// APP MAIN
// =============================
void app_main(void) {
    ESP_LOGI(TAG, "🚀 Expert Challenges System Starting...");
    nvs_flash_init();
    gpio_set_direction(LED_SCHEDULER, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_SYNC, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_LOAD, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_ERROR, GPIO_MODE_OUTPUT);

    init_scheduler();

    // Schedule sample tasks
    schedule_task("TaskA", 500, 1, 200);
    schedule_task("TaskB", 800, 3, 400);
    schedule_task("TaskC", 1000, 5, 500);

    // Create expert-level controllers
    xTaskCreate(network_sync_task, "NetSync", 4096, NULL, 5, NULL);
    xTaskCreate(adaptive_controller_task, "AdaptiveCtrl", 4096, NULL, 6, NULL);

    ESP_LOGI(TAG, "✅ Expert Timer System Initialized");
    ESP_LOGI(TAG, "LED2: Scheduler | LED4: Network Sync | LED5: Load | LED18: Error");
}
