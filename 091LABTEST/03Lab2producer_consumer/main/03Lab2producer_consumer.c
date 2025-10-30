#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_random.h"

static const char *TAG = "LAB2_PROD_CONS";

// GPIO LEDs
#define LED_PRODUCER GPIO_NUM_2
#define LED_CONSUMER GPIO_NUM_4
#define LED_QC GPIO_NUM_5

// Queue handles (แบ่งเป็นหมวดสินค้า)
QueueHandle_t xQueueFood;
QueueHandle_t xQueueDrink;

// Mutex
SemaphoreHandle_t xPrintMutex;

// Stats
typedef struct {
    uint32_t produced;
    uint32_t consumed;
    uint32_t dropped;
    uint32_t qc_failed;
} stats_t;

stats_t global_stats = {0, 0, 0, 0};

// Product structure
typedef struct {
    int id;
    char category[10];
    char name[30];
    int processing_time;
    uint32_t timestamp;
} product_t;

// ---------- Safe print ----------
void safe_printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    if (xSemaphoreTake(xPrintMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        vprintf(fmt, args);
        xSemaphoreGive(xPrintMutex);
    }
    va_end(args);
}

// ---------- Producer ----------
void producer_task(void *pvParams) {
    int producer_id = (int)pvParams;
    int counter = 0;
    safe_printf("🍳 Producer %d started\n", producer_id);

    while (1) {
        product_t item;
        item.id = counter++;
        item.processing_time = 500 + (esp_random() % 2000);
        snprintf(item.name, sizeof(item.name), "Product#%d", item.id);
        strcpy(item.category, (item.id % 2 == 0) ? "Food" : "Drink");
        item.timestamp = xTaskGetTickCount();

        QueueHandle_t targetQueue = (strcmp(item.category, "Food") == 0) ? xQueueFood : xQueueDrink;

        if (xQueueSend(targetQueue, &item, pdMS_TO_TICKS(200)) == pdPASS) {
            global_stats.produced++;
            safe_printf("✅ P%d Produced: %s (%s)\n", producer_id, item.name, item.category);
            gpio_set_level(LED_PRODUCER, 1);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_set_level(LED_PRODUCER, 0);
        } else {
            global_stats.dropped++;
            safe_printf("⚠️ P%d Failed to enqueue (queue full)\n", producer_id);
        }

        vTaskDelay(pdMS_TO_TICKS(1000 + (esp_random() % 1000)));
    }
}

// ---------- Quality Control ----------
void qc_task(void *pvParams) {
    product_t item;
    safe_printf("🔍 QC task started\n");

    while (1) {
        if (xQueueReceive(xQueueFood, &item, pdMS_TO_TICKS(1000)) == pdPASS ||
            xQueueReceive(xQueueDrink, &item, pdMS_TO_TICKS(1000)) == pdPASS) {

            // ตรวจคุณภาพ (20% ไม่ผ่าน)
            if (esp_random() % 5 == 0) {
                global_stats.qc_failed++;
                safe_printf("❌ QC Failed: %s (%s)\n", item.name, item.category);
                continue;
            }

            safe_printf("🧪 QC Passed: %s (%s)\n", item.name, item.category);

            // ส่งไป Consumer ผ่านคิวรวม
            if (xQueueSendToBack((strcmp(item.category, "Food") == 0) ? xQueueFood : xQueueDrink,
                                 &item, pdMS_TO_TICKS(100)) != pdPASS) {
                global_stats.dropped++;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(300));
    }
}

// ---------- Consumer ----------
void consumer_task(void *pvParams) {
    int consumer_id = (int)pvParams;
    product_t batch[3]; // Batch size 3
    safe_printf("👷 Consumer %d started\n", consumer_id);

    while (1) {
        int count = 0;
        QueueHandle_t sourceQueue = (consumer_id % 2 == 0) ? xQueueFood : xQueueDrink;

        while (count < 3 && xQueueReceive(sourceQueue, &batch[count], pdMS_TO_TICKS(1000)) == pdPASS) {
            count++;
        }

        if (count > 0) {
            safe_printf("📦 Consumer %d processing batch (%d items)\n", consumer_id, count);
            gpio_set_level(LED_CONSUMER, 1);
            for (int i = 0; i < count; i++) {
                global_stats.consumed++;
                vTaskDelay(pdMS_TO_TICKS(batch[i].processing_time));
                safe_printf("→ Consumed: %s (%s)\n", batch[i].name, batch[i].category);
            }
            gpio_set_level(LED_CONSUMER, 0);

            // ส่งข้อมูลจำลองไป WiFi
            safe_printf("📡 Consumer %d sent batch to server\n", consumer_id);
        } else {
            safe_printf("⏳ Consumer %d idle\n", consumer_id);
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

// ---------- Dynamic Load Balancer ----------
void load_balancer_task(void *pvParams) {
    static TaskHandle_t dynamic_consumer = NULL;
    int next_id = 3;

    while (1) {
        UBaseType_t food_load = uxQueueMessagesWaiting(xQueueFood);
        UBaseType_t drink_load = uxQueueMessagesWaiting(xQueueDrink);
        int total = food_load + drink_load;

        if (total > 8 && dynamic_consumer == NULL) {
            safe_printf("⚡ High load (%d items), adding extra consumer!\n", total);
            static int cid = 3;
            xTaskCreate(consumer_task, "DynConsumer", 4096, &cid, 2, &dynamic_consumer);
        } else if (total < 3 && dynamic_consumer != NULL) {
            safe_printf("💤 Low load (%d items), removing dynamic consumer.\n", total);
            vTaskDelete(dynamic_consumer);
            dynamic_consumer = NULL;
        }

        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

// ---------- Statistics ----------
void statistics_task(void *pvParams) {
    while (1) {
        UBaseType_t food = uxQueueMessagesWaiting(xQueueFood);
        UBaseType_t drink = uxQueueMessagesWaiting(xQueueDrink);
        safe_printf("\n📊 Stats: Prod=%lu | Cons=%lu | Drop=%lu | QC_Fail=%lu | FoodQ=%d | DrinkQ=%d\n",
                    global_stats.produced, global_stats.consumed, global_stats.dropped,
                    global_stats.qc_failed, food, drink);
        vTaskDelay(pdMS_TO_TICKS(4000));
    }
}

// ---------- Main ----------
void app_main(void) {
    ESP_LOGI(TAG, "🚀 03Lab2 Producer-Consumer with Challenges Starting...");

    gpio_set_direction(LED_PRODUCER, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_CONSUMER, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_QC, GPIO_MODE_OUTPUT);

    xQueueFood = xQueueCreate(10, sizeof(product_t));
    xQueueDrink = xQueueCreate(10, sizeof(product_t));
    xPrintMutex = xSemaphoreCreateMutex();

    if (!xQueueFood || !xQueueDrink || !xPrintMutex) {
        ESP_LOGE(TAG, "❌ Queue or Mutex creation failed!");
        return;
    }

    static int p1 = 1, p2 = 2, c1 = 1, c2 = 2;
    xTaskCreate(producer_task, "Producer1", 4096, &p1, 3, NULL);
    xTaskCreate(producer_task, "Producer2", 4096, &p2, 3, NULL);
    xTaskCreate(qc_task, "QualityControl", 4096, NULL, 2, NULL);
    xTaskCreate(consumer_task, "Consumer1", 4096, &c1, 2, NULL);
    xTaskCreate(consumer_task, "Consumer2", 4096, &c2, 2, NULL);
    xTaskCreate(load_balancer_task, "LoadBalancer", 4096, NULL, 1, NULL);
    xTaskCreate(statistics_task, "Statistics", 3072, NULL, 1, NULL);

    ESP_LOGI(TAG, "✅ All tasks created successfully!");
}
