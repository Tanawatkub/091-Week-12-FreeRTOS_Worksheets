#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_timer.h"

static const char *TAG = "LOGGING_DEMO";


// ======================= Exercise 1: Custom Logger ==========================
#define LOG_COLOR_CYAN "36"
#define LOG_BOLD(COLOR)  "\033[1;" COLOR "m"
#define LOG_RESET_COLOR  "\033[0m"

void custom_log(const char* tag, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    printf(LOG_BOLD(LOG_COLOR_CYAN) "[CUSTOM] %s: " LOG_RESET_COLOR, tag);
    vprintf(format, args);
    printf("\n");
    va_end(args);
}


// ======================= Exercise 2: Performance Monitoring =================
void performance_demo(void)
{
    ESP_LOGI(TAG, "=== Performance Monitoring ===");

    uint64_t start_time = esp_timer_get_time();

    // Simulate some work
    for (int i = 0; i < 1000000; i++) {
        volatile int dummy = i * 2;
    }

    uint64_t end_time = esp_timer_get_time();
    uint64_t exec_time = end_time - start_time;

    ESP_LOGI(TAG, "Execution time: %lld microseconds", exec_time);
    ESP_LOGI(TAG, "Execution time: %.2f milliseconds", exec_time / 1000.0);
}


// ======================= Exercise 3: Error Handling ========================
void error_handling_demo(void)
{
    ESP_LOGI(TAG, "=== Error Handling Demo ===");

    esp_err_t result;

    // Case 1: Success
    result = ESP_OK;
    if (result == ESP_OK) {
        ESP_LOGI(TAG, "Operation completed successfully");
    }

    // Case 2: Simulate out of memory
    result = ESP_ERR_NO_MEM;
    if (result != ESP_OK) {
        ESP_LOGE(TAG, "Error: %s", esp_err_to_name(result));
    }

    // Case 3: Non-fatal error
    result = ESP_ERR_INVALID_ARG;
    ESP_ERROR_CHECK_WITHOUT_ABORT(result);
    if (result != ESP_OK) {
        ESP_LOGW(TAG, "Non-fatal error: %s", esp_err_to_name(result));
    }
}


// ======================= Main Program =====================================
void app_main(void)
{
    // Initialize NVS (Non-Volatile Storage)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Basic System Info
    ESP_LOGI(TAG, "=== ESP32 Hello World & Logging Demo ===");
    ESP_LOGI(TAG, "ESP-IDF Version: %s", esp_get_idf_version());
    ESP_LOGI(TAG, "Chip: %s", CONFIG_IDF_TARGET);
    ESP_LOGI(TAG, "Free Heap: %d bytes", esp_get_free_heap_size());

    // Exercise 1
    ESP_LOGI(TAG, "\n--- Running Custom Logger ---");
    custom_log("SENSOR", "Temperature: %d°C", 25);

    // Exercise 2
    ESP_LOGI(TAG, "\n--- Running Performance Demo ---");
    performance_demo();

    // Exercise 3
    ESP_LOGI(TAG, "\n--- Running Error Handling Demo ---");
    error_handling_demo();

    // Main Loop (keep system alive)
    int counter = 0;
    while (1) {
        ESP_LOGI(TAG, "Main loop iteration: %d", counter++);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
