#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/gpio.h"

static const char *TAG = "QUEUE_ADV";

// LED pins
#define LED_SENDER_1 GPIO_NUM_2
#define LED_SENDER_2 GPIO_NUM_5
#define LED_RECEIVER GPIO_NUM_4

// Queue size (ปรับได้ตามต้องการ)
#define QUEUE_LENGTH 5

// Data structure for queue messages
typedef struct {
    int id;
    int priority; // 0 = low, 1 = high
    char message[50];
    uint32_t timestamp;
} queue_message_t;

// Queue handle
QueueHandle_t xQueue;

// Statistics
typedef struct {
    uint32_t sent;
    uint32_t received;
    uint32_t dropped;
} stats_t;

stats_t global_stats = {0, 0, 0};

// -------- Safe LED blink ----------
void blink_led(gpio_num_t led, int duration_ms) {
    gpio_set_level(led, 1);
    vTaskDelay(pdMS_TO_TICKS(duration_ms));
    gpio_set_level(led, 0);
}

// -------- Sender Task -------------
void sender_task(void *pvParameters) {
    int sender_id = (int)pvParameters;
    gpio_num_t led_pin = (sender_id == 1) ? LED_SENDER_1 : LED_SENDER_2;
    int counter = 0;

    ESP_LOGI(TAG, "Sender %d started", sender_id);

    while (1) {
        queue_message_t msg;
        msg.id = counter++;
        msg.priority = (msg.id % 3 == 0) ? 1 : 0; // ทุก 3 ข้อความเป็น High Priority
        snprintf(msg.message, sizeof(msg.message),
                 "Sender%d -> MSG#%d (P%d)", sender_id, msg.id, msg.priority);
        msg.timestamp = xTaskGetTickCount();

        BaseType_t xStatus = xQueueSend(xQueue, &msg, pdMS_TO_TICKS(500));

        if (xStatus == pdPASS) {
            global_stats.sent++;
            ESP_LOGI(TAG, "✅ Sender%d Sent: %s", sender_id, msg.message);
            blink_led(led_pin, 100);
        } else {
            global_stats.dropped++;
            ESP_LOGW(TAG, "⚠️ Sender%d failed to send (Queue full)", sender_id);
        }

        // ส่งเร็วช้าไม่เท่ากัน
        vTaskDelay(pdMS_TO_TICKS(sender_id == 1 ? 1500 : 2200));
    }
}

// -------- Receiver Task ------------
void receiver_task(void *pvParameters) {
    queue_message_t received[QUEUE_LENGTH];
    ESP_LOGI(TAG, "Receiver started");

    while (1) {
        int received_count = 0;
        // อ่านข้อความทั้งหมดที่อยู่ในคิวตอนนี้
        while (uxQueueMessagesWaiting(xQueue) > 0 &&
               received_count < QUEUE_LENGTH) {
            if (xQueueReceive(xQueue, &received[received_count],
                              pdMS_TO_TICKS(200)) == pdPASS) {
                received_count++;
            }
        }

        if (received_count > 0) {
            // หา priority สูงสุด
            int high_index = 0;
            for (int i = 1; i < received_count; i++) {
                if (received[i].priority > received[high_index].priority)
                    high_index = i;
            }

            queue_message_t msg = received[high_index];
            global_stats.received++;

            ESP_LOGI(TAG, "🎯 Received: %s | Time=%lu", msg.message, msg.timestamp);
            blink_led(LED_RECEIVER, 200);
            vTaskDelay(pdMS_TO_TICKS(1500)); // simulate work
        } else {
            ESP_LOGW(TAG, "⌛ Receiver timeout - No message");
        }
    }
}

// -------- Queue Monitor Task -------
void queue_monitor_task(void *pvParameters) {
    while (1) {
        UBaseType_t used = uxQueueMessagesWaiting(xQueue);
        UBaseType_t free = uxQueueSpacesAvailable(xQueue);

        ESP_LOGI(TAG,
                 "\n📊 Queue Status → Used: %d / Free: %d\nSent=%lu | Received=%lu | Dropped=%lu",
                 used, free,
                 global_stats.sent,
                 global_stats.received,
                 global_stats.dropped);

        printf("Queue Visualization: [");
        for (int i = 0; i < QUEUE_LENGTH; i++) {
            printf(i < used ? "■" : "□");
        }
        printf("]\n");

        vTaskDelay(pdMS_TO_TICKS(4000));
    }
}

// -------- app_main ----------------
void app_main(void) {
    ESP_LOGI(TAG, "🚀 Queue Lab Advanced Starting...");

    gpio_set_direction(LED_SENDER_1, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_SENDER_2, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_RECEIVER, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_SENDER_1, 0);
    gpio_set_level(LED_SENDER_2, 0);
    gpio_set_level(LED_RECEIVER, 0);

    // ✅ Create Queue
    xQueue = xQueueCreate(QUEUE_LENGTH, sizeof(queue_message_t));
    if (xQueue == NULL) {
        ESP_LOGE(TAG, "❌ Failed to create queue!");
        return;
    }

    // ✅ Create Tasks
    xTaskCreate(sender_task, "Sender1", 4096, (void *)1, 2, NULL);
    xTaskCreate(sender_task, "Sender2", 4096, (void *)2, 2, NULL);
    xTaskCreate(receiver_task, "Receiver", 4096, NULL, 2, NULL);
    xTaskCreate(queue_monitor_task, "Monitor", 3072, NULL, 1, NULL);
}
