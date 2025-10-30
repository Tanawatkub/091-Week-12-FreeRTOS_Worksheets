#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"

#define LED_HIGH_PIN GPIO_NUM_2
#define LED_MED_PIN GPIO_NUM_4
#define LED_LOW_PIN GPIO_NUM_5
#define BUTTON_PIN GPIO_NUM_0

static const char *TAG = "LAB1_PRIORITY";

// Global flags
volatile bool priority_test_running = false;
volatile uint32_t high_task_count = 0;
volatile uint32_t med_task_count = 0;
volatile uint32_t low_task_count = 0;
volatile bool shared_resource_busy = false;

// ---------------- HIGH PRIORITY TASK ----------------
void high_priority_task(void *pvParameters)
{
    ESP_LOGI(TAG, "High Priority Task started (Priority 5)");
    while (1)
    {
        if (priority_test_running)
        {
            high_task_count++;
            gpio_set_level(LED_HIGH_PIN, 1);
            for (int i = 0; i < 100000; i++)
                volatile int dummy = i * 2;
            gpio_set_level(LED_HIGH_PIN, 0);
            vTaskDelay(pdMS_TO_TICKS(200));
        }
        else
            vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// ---------------- MEDIUM PRIORITY TASK ----------------
void medium_priority_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Medium Priority Task started (Priority 3)");
    while (1)
    {
        if (priority_test_running)
        {
            med_task_count++;
            gpio_set_level(LED_MED_PIN, 1);
            for (int i = 0; i < 200000; i++)
                volatile int dummy = i + 100;
            gpio_set_level(LED_MED_PIN, 0);
            vTaskDelay(pdMS_TO_TICKS(300));
        }
        else
            vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// ---------------- LOW PRIORITY TASK ----------------
void low_priority_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Low Priority Task started (Priority 1)");
    while (1)
    {
        if (priority_test_running)
        {
            low_task_count++;
            gpio_set_level(LED_LOW_PIN, 1);
            for (int i = 0; i < 500000; i++)
            {
                volatile int dummy = i - 50;
                if (i % 100000 == 0)
                    vTaskDelay(1);
            }
            gpio_set_level(LED_LOW_PIN, 0);
            vTaskDelay(pdMS_TO_TICKS(500));
        }
        else
            vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// ---------------- CONTROL TASK ----------------
void control_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Control Task started");
    while (1)
    {
        if (gpio_get_level(BUTTON_PIN) == 0)
        {
            if (!priority_test_running)
            {
                ESP_LOGW(TAG, "=== STARTING PRIORITY TEST ===");
                high_task_count = 0;
                med_task_count = 0;
                low_task_count = 0;
                priority_test_running = true;

                vTaskDelay(pdMS_TO_TICKS(10000)); // Run for 10 sec
                priority_test_running = false;

                ESP_LOGW(TAG, "=== TEST RESULTS ===");
                ESP_LOGI(TAG, "High: %d  Med: %d  Low: %d",
                         high_task_count, med_task_count, low_task_count);

                uint32_t total = high_task_count + med_task_count + low_task_count;
                if (total > 0)
                {
                    ESP_LOGI(TAG, "High %%: %.1f  Med %%: %.1f  Low %%: %.1f",
                             (float)high_task_count / total * 100,
                             (float)med_task_count / total * 100,
                             (float)low_task_count / total * 100);
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// ---------------- ROUND ROBIN TASKS ----------------
void equal_priority_task(void *pvParameters)
{
    int id = (int)pvParameters;
    while (1)
    {
        if (priority_test_running)
        {
            ESP_LOGI(TAG, "Equal Priority Task %d running", id);
            for (int i = 0; i < 300000; i++)
                volatile int dummy = i;
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// ---------------- PRIORITY INVERSION DEMO ----------------
void priority_inversion_high(void *pvParameters)
{
    while (1)
    {
        if (priority_test_running)
        {
            ESP_LOGW(TAG, "High priority needs resource");
            while (shared_resource_busy)
            {
                ESP_LOGW(TAG, "High blocked by low priority!");
                vTaskDelay(pdMS_TO_TICKS(10));
            }
            ESP_LOGI(TAG, "High got resource");
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void priority_inversion_low(void *pvParameters)
{
    while (1)
    {
        if (priority_test_running)
        {
            ESP_LOGI(TAG, "Low using shared resource");
            shared_resource_busy = true;
            vTaskDelay(pdMS_TO_TICKS(2000));
            shared_resource_busy = false;
            ESP_LOGI(TAG, "Low released resource");
        }
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

// ---------------- DYNAMIC PRIORITY DEMO ----------------
void dynamic_priority_demo(void *pvParameters)
{
    TaskHandle_t low_task_handle = (TaskHandle_t)pvParameters;
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(5000));
        ESP_LOGW(TAG, "Boosting low priority to 4");
        vTaskPrioritySet(low_task_handle, 4);
        vTaskDelay(pdMS_TO_TICKS(2000));
        ESP_LOGW(TAG, "Restoring low priority to 1");
        vTaskPrioritySet(low_task_handle, 1);
    }
}

// ---------------- APP MAIN ----------------
void app_main(void)
{
    ESP_LOGI(TAG, "=== FreeRTOS Task Priority and Scheduling Lab ===");

    // Setup LEDs
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << LED_HIGH_PIN) | (1ULL << LED_MED_PIN) | (1ULL << LED_LOW_PIN),
        .pull_down_en = 0,
        .pull_up_en = 0};
    gpio_config(&io_conf);

    // Button
    gpio_config_t btn_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = 1ULL << BUTTON_PIN,
        .pull_up_en = 1,
        .pull_down_en = 0};
    gpio_config(&btn_conf);

    // Create tasks
    TaskHandle_t low_handle = NULL;
    xTaskCreate(high_priority_task, "High", 3072, NULL, 5, NULL);
    xTaskCreate(medium_priority_task, "Med", 3072, NULL, 3, NULL);
    xTaskCreate(low_priority_task, "Low", 3072, NULL, 1, &low_handle);
    xTaskCreate(control_task, "Control", 3072, NULL, 4, NULL);

    // Round robin test
    xTaskCreate(equal_priority_task, "Equal1", 2048, (void *)1, 2, NULL);
    xTaskCreate(equal_priority_task, "Equal2", 2048, (void *)2, 2, NULL);
    xTaskCreate(equal_priority_task, "Equal3", 2048, (void *)3, 2, NULL);

    // Priority inversion demo
    xTaskCreate(priority_inversion_high, "InvHigh", 3072, NULL, 5, NULL);
    xTaskCreate(priority_inversion_low, "InvLow", 3072, NULL, 1, NULL);

    // Dynamic priority change
    xTaskCreate(dynamic_priority_demo, "Dynamic", 3072, low_handle, 3, NULL);

    ESP_LOGI(TAG, "Press the button to start the test!");
}
