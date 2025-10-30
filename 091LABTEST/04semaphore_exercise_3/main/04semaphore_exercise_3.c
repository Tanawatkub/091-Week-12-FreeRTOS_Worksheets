#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"

static const char *TAG = "EX3_CONN_POOL";

// ----------------------------
// ⚙️ CONFIGURATION
// ----------------------------
#define MAX_CONNECTIONS 3
#define NUM_CLIENTS 5

// ----------------------------
// 🧩 STRUCTURES
// ----------------------------
typedef struct {
    int id;
    bool in_use;
} connection_t;

connection_t connections[MAX_CONNECTIONS];
SemaphoreHandle_t pool_semaphore;
SemaphoreHandle_t pool_mutex;  // เพื่อป้องกันการแก้ไข array พร้อมกัน

// ----------------------------
// 🧠 Utility Functions
// ----------------------------
connection_t *acquire_connection(void)
{
    // รอจนกว่าจะมี connection ว่าง
    if (xSemaphoreTake(pool_semaphore, pdMS_TO_TICKS(5000)) == pdTRUE) {
        // เข้าสู่ Critical Section
        if (xSemaphoreTake(pool_mutex, pdMS_TO_TICKS(500)) == pdTRUE) {
            for (int i = 0; i < MAX_CONNECTIONS; i++) {
                if (!connections[i].in_use) {
                    connections[i].in_use = true;
                    ESP_LOGI(TAG, "✅ Acquired connection #%d", connections[i].id);
                    xSemaphoreGive(pool_mutex);
                    return &connections[i];
                }
            }
            xSemaphoreGive(pool_mutex);
        }
    } else {
        ESP_LOGW(TAG, "⚠️  Timeout waiting for connection!");
    }
    return NULL;
}

void release_connection(connection_t *conn)
{
    if (conn == NULL) return;

    if (xSemaphoreTake(pool_mutex, pdMS_TO_TICKS(500)) == pdTRUE) {
        conn->in_use = false;
        ESP_LOGI(TAG, "🔓 Released connection #%d back to pool", conn->id);
        xSemaphoreGive(pool_mutex);
    }

    // ปล่อย semaphore กลับคืน (เพิ่มจำนวน connection ที่ว่าง)
    xSemaphoreGive(pool_semaphore);
}

void print_pool_status(void)
{
    if (xSemaphoreTake(pool_mutex, pdMS_TO_TICKS(500)) == pdTRUE) {
        printf("📊 Pool Status: ");
        for (int i = 0; i < MAX_CONNECTIONS; i++) {
            if (connections[i].in_use)
                printf("[#%d:🟢] ", connections[i].id);
            else
                printf("[#%d:⚪️] ", connections[i].id);
        }
        printf("\n");
        xSemaphoreGive(pool_mutex);
    }
}

// ----------------------------
// 👤 Client Task
// ----------------------------
void client_task(void *param)
{
    int client_id = (int)param;
    connection_t *conn;

    while (1) {
        ESP_LOGI(TAG, "Client %d requesting connection...", client_id);

        conn = acquire_connection();
        if (conn) {
            ESP_LOGI(TAG, "Client %d connected using #%d", client_id, conn->id);
            print_pool_status();

            // จำลองการใช้งาน connection 1–4 วินาที
            vTaskDelay(pdMS_TO_TICKS(1000 + (rand() % 3000)));

            // ปล่อย connection
            release_connection(conn);
            print_pool_status();
        } else {
            ESP_LOGW(TAG, "Client %d could not get connection", client_id);
        }

        // รอ 2–5 วินาทีก่อนลองใหม่
        vTaskDelay(pdMS_TO_TICKS(2000 + (rand() % 3000)));
    }
}

// ----------------------------
// 🚀 app_main()
// ----------------------------
void app_main(void)
{
    ESP_LOGI(TAG, "=== Exercise 3: Connection Pool Manager ===");

    // สร้าง connection pool
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        connections[i].id = i + 1;
        connections[i].in_use = false;
    }

    pool_semaphore = xSemaphoreCreateCounting(MAX_CONNECTIONS, MAX_CONNECTIONS);
    pool_mutex = xSemaphoreCreateMutex();

    if (!pool_semaphore || !pool_mutex) {
        ESP_LOGE(TAG, "Failed to create semaphores!");
        return;
    }

    // สร้าง client หลายตัว
    for (int i = 1; i <= NUM_CLIENTS; i++) {
        xTaskCreate(client_task, "Client", 3072, (void *)i, 5, NULL);
    }

    ESP_LOGI(TAG, "Connection Pool initialized with %d connections", MAX_CONNECTIONS);
}
