#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_system.h"

#define LED_OK GPIO_NUM_2
#define LED_WARNING GPIO_NUM_4

static const char *TAG = "STACK_MONITOR";

#define STACK_WARNING_THRESHOLD 512
#define STACK_CRITICAL_THRESHOLD 256

TaskHandle_t light_task_handle = NULL;
TaskHandle_t medium_task_handle = NULL;
TaskHandle_t heavy_task_handle = NULL;

// ====================== TASKS ======================

// Stack monitor
void stack_monitor_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Stack Monitor Task started");

    while (1)
    {
        ESP_LOGI(TAG, "\n=== STACK USAGE REPORT ===");

        TaskHandle_t tasks[] = {
            light_task_handle,
            medium_task_handle,
            heavy_task_handle,
            xTaskGetCurrentTaskHandle() // monitor itself
        };

        const char *names[] = {
            "LightTask",
            "MediumTask",
            "HeavyTask",
            "StackMonitor"};

        bool stack_warning = false;
        bool stack_critical = false;

        for (int i = 0; i < 4; i++)
        {
            if (tasks[i] != NULL)
            {
                UBaseType_t remain = uxTaskGetStackHighWaterMark(tasks[i]);
                uint32_t bytes = remain * sizeof(StackType_t);

                ESP_LOGI(TAG, "%s: %d bytes remaining", names[i], bytes);

                if (bytes < STACK_CRITICAL_THRESHOLD)
                {
                    ESP_LOGE(TAG, "CRITICAL: %s stack very low!", names[i]);
                    stack_critical = true;
                }
                else if (bytes < STACK_WARNING_THRESHOLD)
                {
                    ESP_LOGW(TAG, "WARNING: %s stack low", names[i]);
                    stack_warning = true;
                }
            }
        }

        if (stack_critical)
        {
            for (int i = 0; i < 10; i++)
            {
                gpio_set_level(LED_WARNING, 1);
                vTaskDelay(pdMS_TO_TICKS(50));
                gpio_set_level(LED_WARNING, 0);
                vTaskDelay(pdMS_TO_TICKS(50));
            }
            gpio_set_level(LED_OK, 0);
        }
        else if (stack_warning)
        {
            gpio_set_level(LED_WARNING, 1);
            gpio_set_level(LED_OK, 0);
        }
        else
        {
            gpio_set_level(LED_OK, 1);
            gpio_set_level(LED_WARNING, 0);
        }

        ESP_LOGI(TAG, "Free heap: %d bytes", esp_get_free_heap_size());
        ESP_LOGI(TAG, "Min free heap: %d bytes", esp_get_minimum_free_heap_size());

        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

void light_stack_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Light Stack Task started");

    int counter = 0;

    while (1)
    {
        counter++;
        ESP_LOGI(TAG, "Light task cycle %d", counter);
        UBaseType_t remain = uxTaskGetStackHighWaterMark(NULL);
        ESP_LOGD(TAG, "Light stack: %d bytes", remain * sizeof(StackType_t));
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void medium_stack_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Medium Stack Task started");

    while (1)
    {
        char buffer[256];
        int nums[50];
        memset(buffer, 'A', sizeof(buffer) - 1);
        buffer[255] = '\0';
        for (int i = 0; i < 50; i++)
            nums[i] = i * i;

        ESP_LOGI(TAG, "Medium: %c %d", buffer[0], nums[49]);
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

void heavy_stack_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Heavy Stack Task started");

    int cycle = 0;
    while (1)
    {
        cycle++;
        char buf1[1024];
        int nums[200];
        char buf2[512];
        memset(buf1, 'X', sizeof(buf1) - 1);
        buf1[1023] = '\0';
        for (int i = 0; i < 200; i++)
            nums[i] = i * cycle;

        snprintf(buf2, sizeof(buf2), "Cycle %d", cycle);

        UBaseType_t remain = uxTaskGetStackHighWaterMark(NULL);
        uint32_t bytes = remain * sizeof(StackType_t);
        if (bytes < STACK_CRITICAL_THRESHOLD)
            ESP_LOGE(TAG, "DANGER: Heavy task critically low (%d bytes)", bytes);
        else
            ESP_LOGW(TAG, "Heavy stack remaining: %d bytes", bytes);

        vTaskDelay(pdMS_TO_TICKS(4000));
    }
}

// Recursive demo
void recursive_function(int depth)
{
    char local[100];
    snprintf(local, sizeof(local), "Recursion depth %d", depth);
    ESP_LOGI(TAG, "%s", local);

    UBaseType_t remain = uxTaskGetStackHighWaterMark(NULL);
    uint32_t bytes = remain * sizeof(StackType_t);
    ESP_LOGI(TAG, "Depth %d: Stack %d bytes", depth, bytes);

    if (bytes < 200)
    {
        ESP_LOGE(TAG, "Stopping recursion at depth %d", depth);
        return;
    }

    if (depth < 20)
    {
        vTaskDelay(pdMS_TO_TICKS(500));
        recursive_function(depth + 1);
    }
}

void recursion_demo_task(void *pvParameters)
{
    while (1)
    {
        ESP_LOGW(TAG, "=== START RECURSION ===");
        recursive_function(1);
        ESP_LOGW(TAG, "=== END RECURSION ===");
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

// Optimized version
void optimized_heavy_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Optimized Heavy Task using heap");

    char *buf = malloc(1024);
    int *nums = malloc(200 * sizeof(int));
    char *b2 = malloc(512);
    if (!buf || !nums || !b2)
    {
        ESP_LOGE(TAG, "Heap allocation failed");
        vTaskDelete(NULL);
        return;
    }

    int c = 0;
    while (1)
    {
        c++;
        memset(buf, 'Y', 1023);
        buf[1023] = '\0';
        for (int i = 0; i < 200; i++)
            nums[i] = i * c;

        snprintf(b2, 512, "Optimized cycle %d", c);
        UBaseType_t remain = uxTaskGetStackHighWaterMark(NULL);
        ESP_LOGI(TAG, "Optimized task stack: %d bytes", remain * sizeof(StackType_t));
        vTaskDelay(pdMS_TO_TICKS(4000));
    }
}

// Stack overflow hook
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    ESP_LOGE("STACK_OVERFLOW", "Task %s overflowed its stack!", pcTaskName);
    for (int i = 0; i < 10; i++)
    {
        gpio_set_level(LED_WARNING, 1);
        vTaskDelay(pdMS_TO_TICKS(50));
        gpio_set_level(LED_WARNING, 0);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    esp_restart();
}

// ====================== APP MAIN ======================

void app_main(void)
{
    gpio_config_t conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << LED_OK) | (1ULL << LED_WARNING)};
    gpio_config(&conf);

    ESP_LOGI(TAG, "GPIO2 = OK, GPIO4 = Warning");

    xTaskCreate(light_stack_task, "LightTask", 1024, NULL, 2, &light_task_handle);
    xTaskCreate(medium_stack_task, "MediumTask", 2048, NULL, 2, &medium_task_handle);
    xTaskCreate(heavy_stack_task, "HeavyTask", 2048, NULL, 2, &heavy_task_handle);
    xTaskCreate(recursion_demo_task, "RecursionDemo", 3072, NULL, 1, NULL);
    xTaskCreate(stack_monitor_task, "StackMonitor", 4096, NULL, 3, NULL);
    xTaskCreate(optimized_heavy_task, "OptHeavy", 3072, NULL, 2, NULL);
}
