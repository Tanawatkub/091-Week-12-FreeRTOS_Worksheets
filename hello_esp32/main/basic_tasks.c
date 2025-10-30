#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/semphr.h"

#define LED1_PIN GPIO_NUM_2
#define LED2_PIN GPIO_NUM_4

static const char *TAG = "BASIC_TASKS";

// ================= Step 1: Basic LED Tasks ==================
void led1_task(void *pvParameters)
{
    int *task_id = (int *)pvParameters;
    ESP_LOGI(TAG, "LED1 Task started with ID: %d", *task_id);

    while (1)
    {
        ESP_LOGI(TAG, "LED1 ON");
        gpio_set_level(LED1_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(500));

        ESP_LOGI(TAG, "LED1 OFF");
        gpio_set_level(LED1_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void led2_task(void *pvParameters)
{
    char *task_name = (char *)pvParameters;
    ESP_LOGI(TAG, "LED2 Task started: %s", task_name);

    while (1)
    {
        ESP_LOGI(TAG, "LED2 Blink Fast");
        for (int i = 0; i < 5; i++)
        {
            gpio_set_level(LED2_PIN, 1);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_set_level(LED2_PIN, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void system_info_task(void *pvParameters)
{
    while (1)
    {
        ESP_LOGI(TAG, "=== System Information ===");
        ESP_LOGI(TAG, "Free heap: %d bytes", esp_get_free_heap_size());
        ESP_LOGI(TAG, "Min free heap: %d bytes", esp_get_minimum_free_heap_size());
        UBaseType_t task_count = uxTaskGetNumberOfTasks();
        ESP_LOGI(TAG, "Number of tasks: %d", task_count);
        TickType_t uptime = xTaskGetTickCount();
        ESP_LOGI(TAG, "Uptime: %lu seconds", (uptime * portTICK_PERIOD_MS) / 1000);
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

// ================= Step 2: Task Manager ==================
void task_manager(void *pvParameters)
{
    TaskHandle_t *handles = (TaskHandle_t *)pvParameters;
    TaskHandle_t led1_handle = handles[0];
    TaskHandle_t led2_handle = handles[1];

    int command_counter = 0;

    while (1)
    {
        command_counter++;

        switch (command_counter % 6)
        {
        case 1:
            ESP_LOGI(TAG, "Manager: Suspending LED1");
            vTaskSuspend(led1_handle);
            break;
        case 2:
            ESP_LOGI(TAG, "Manager: Resuming LED1");
            vTaskResume(led1_handle);
            break;
        case 3:
            ESP_LOGI(TAG, "Manager: Suspending LED2");
            vTaskSuspend(led2_handle);
            break;
        case 4:
            ESP_LOGI(TAG, "Manager: Resuming LED2");
            vTaskResume(led2_handle);
            break;
        case 5:
            ESP_LOGI(TAG, "Manager: Getting task info");
            ESP_LOGI(TAG, "LED1 State: %d", eTaskGetState(led1_handle));
            ESP_LOGI(TAG, "LED2 State: %d", eTaskGetState(led2_handle));
            break;
        default:
            ESP_LOGI(TAG, "Manager: Reset cycle");
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

// ================= Step 3: Priority & Runtime Stats ==================
void high_priority_task(void *pvParameters)
{
    ESP_LOGW(TAG, "High Priority Task started");

    while (1)
    {
        ESP_LOGW(TAG, "HIGH PRIORITY TASK RUNNING!");
        for (int i = 0; i < 1000000; i++)
            volatile int dummy = i;
        ESP_LOGW(TAG, "High priority task yielding");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void low_priority_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Low Priority Task started");

    while (1)
    {
        for (int i = 0; i < 50; i++)
        {
            ESP_LOGI(TAG, "Low priority task running... %d", i + 1);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

void runtime_stats_task(void *pvParameters)
{
    char *buffer = malloc(1024);
    if (!buffer)
    {
        ESP_LOGE(TAG, "Failed to allocate buffer");
        vTaskDelete(NULL);
    }

    while (1)
    {
        ESP_LOGI(TAG, "\n=== Runtime Statistics ===");
        vTaskGetRunTimeStats(buffer);
        ESP_LOGI(TAG, "Task\tAbs Time\tPercent Time");
        ESP_LOGI(TAG, "%s", buffer);

        ESP_LOGI(TAG, "\n=== Task List ===");
        vTaskList(buffer);
        ESP_LOGI(TAG, "Name\tState\tPrio\tStack\tNum");
        ESP_LOGI(TAG, "%s", buffer);

        vTaskDelay(pdMS_TO_TICKS(10000));
    }

    free(buffer);
}

// ================= Exercise 1: Task Self-Deletion ==================
void temporary_task(void *pvParameters)
{
    int *duration = (int *)pvParameters;
    ESP_LOGI(TAG, "Temporary task will run for %d seconds", *duration);

    for (int i = *duration; i > 0; i--)
    {
        ESP_LOGI(TAG, "Temporary task countdown: %d", i);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    ESP_LOGI(TAG, "Temporary task self-deleting");
    vTaskDelete(NULL);
}

// ================= Exercise 2: Simple Task Communication ==================
volatile int shared_counter = 0;

void producer_task(void *pvParameters)
{
    while (1)
    {
        shared_counter++;
        ESP_LOGI(TAG, "Producer: counter = %d", shared_counter);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void consumer_task(void *pvParameters)
{
    int last_value = 0;
    while (1)
    {
        if (shared_counter != last_value)
        {
            ESP_LOGI(TAG, "Consumer: received %d", shared_counter);
            last_value = shared_counter;
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

// ================= Main Function ==================
void app_main(void)
{
    ESP_LOGI(TAG, "=== FreeRTOS Basic Tasks Demo ===");

    // GPIO Setup
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << LED1_PIN) | (1ULL << LED2_PIN),
        .pull_up_en = 0,
        .pull_down_en = 0,
        .intr_type = GPIO_INTR_DISABLE};
    gpio_config(&io_conf);

    // Parameters
    static int led1_id = 1;
    static char led2_name[] = "FastBlinker";
    static int temp_duration = 10;

    // Create Basic Tasks
    TaskHandle_t led1_handle = NULL, led2_handle = NULL, info_handle = NULL;

    xTaskCreate(led1_task, "LED1_Task", 2048, &led1_id, 2, &led1_handle);
    xTaskCreate(led2_task, "LED2_Task", 2048, led2_name, 2, &led2_handle);
    xTaskCreate(system_info_task, "SysInfo_Task", 3072, NULL, 1, &info_handle);

    // Step 2: Manager
    TaskHandle_t handles[2] = {led1_handle, led2_handle};
    xTaskCreate(task_manager, "TaskManager", 2048, handles, 3, NULL);

    // Step 3: Priority & Runtime Stats
    xTaskCreate(high_priority_task, "HighPriority", 3072, NULL, 4, NULL);
    xTaskCreate(low_priority_task, "LowPriority", 3072, NULL, 1, NULL);
    xTaskCreate(runtime_stats_task, "RuntimeStats", 4096, NULL, 1, NULL);

    // Exercise 1: Temporary Task
    xTaskCreate(temporary_task, "TempTask", 2048, &temp_duration, 1, NULL);

    // Exercise 2: Task Communication
    xTaskCreate(producer_task, "Producer", 2048, NULL, 2, NULL);
    xTaskCreate(consumer_task, "Consumer", 2048, NULL, 2, NULL);

    // Main Task
    while (1)
    {
        ESP_LOGI(TAG, "Main task heartbeat");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
