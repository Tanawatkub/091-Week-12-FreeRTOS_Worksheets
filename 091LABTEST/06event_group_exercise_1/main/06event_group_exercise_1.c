#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"

static const char *TAG = "EVENT_EXERCISES";
EventGroupHandle_t event_group;

// ============================ COMMON UTILITIES ============================
static void random_delay(int min_ms, int max_ms) {
    vTaskDelay(pdMS_TO_TICKS(min_ms + (rand() % (max_ms - min_ms))));
}

// ============================ EXERCISE 1 ============================
#define HW_INIT_BIT   BIT0
#define NET_INIT_BIT  BIT1
#define CFG_INIT_BIT  BIT2
#define ALL_INIT_BITS (HW_INIT_BIT | NET_INIT_BIT | CFG_INIT_BIT)

void hw_init_task(void *p) {
    ESP_LOGI(TAG, "Hardware init started");
    random_delay(1000, 5000);
    ESP_LOGI(TAG, "Hardware init done");
    xEventGroupSetBits(event_group, HW_INIT_BIT);
    vTaskDelete(NULL);
}

void net_init_task(void *p) {
    ESP_LOGI(TAG, "Network init started");
    random_delay(2000, 6000);
    ESP_LOGI(TAG, "Network init done");
    xEventGroupSetBits(event_group, NET_INIT_BIT);
    vTaskDelete(NULL);
}

void cfg_init_task(void *p) {
    ESP_LOGI(TAG, "Config init started");
    random_delay(1000, 4000);
    ESP_LOGI(TAG, "Config init done");
    xEventGroupSetBits(event_group, CFG_INIT_BIT);
    vTaskDelete(NULL);
}

void main_task_ex1(void *p) {
    ESP_LOGI(TAG, "Waiting for all initialization...");
    EventBits_t bits = xEventGroupWaitBits(event_group, ALL_INIT_BITS,
                                           pdFALSE, pdTRUE, pdMS_TO_TICKS(10000));

    if ((bits & ALL_INIT_BITS) == ALL_INIT_BITS)
        ESP_LOGI(TAG, "✅ All systems initialized successfully!");
    else
        ESP_LOGW(TAG, "⚠️ Initialization timeout! Received bits=0x%02x", bits);

    vTaskDelete(NULL);
}

void start_exercise1(void) {
    ESP_LOGI(TAG, "===== EXERCISE 1: Basic Event Coordination =====");
    xTaskCreate(hw_init_task, "HW", 2048, NULL, 5, NULL);
    xTaskCreate(net_init_task, "NET", 2048, NULL, 5, NULL);
    xTaskCreate(cfg_init_task, "CFG", 2048, NULL, 5, NULL);
    xTaskCreate(main_task_ex1, "MAIN1", 2048, NULL, 4, NULL);
}

// ============================ EXERCISE 2 ============================
#define TEMP_BIT BIT0
#define HUM_BIT  BIT1
#define PRES_BIT BIT2
#define ALERT_BIT BIT3
#define BASIC_ENV (TEMP_BIT | HUM_BIT)
#define FULL_ENV  (BASIC_ENV | PRES_BIT)

typedef struct {
    float temp, hum, pres;
} sensor_data_t;

sensor_data_t sensor_data;

void temp_task(void *p) {
    while (1) {
        sensor_data.temp = 20 + (rand() % 100) / 5.0;
        ESP_LOGI(TAG, "Temp = %.1f", sensor_data.temp);
        xEventGroupSetBits(event_group, TEMP_BIT);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void hum_task(void *p) {
    while (1) {
        sensor_data.hum = 40 + (rand() % 200) / 5.0;
        ESP_LOGI(TAG, "Hum  = %.1f%%", sensor_data.hum);
        xEventGroupSetBits(event_group, HUM_BIT);
        vTaskDelay(pdMS_TO_TICKS(1200));
    }
}

void pres_task(void *p) {
    while (1) {
        sensor_data.pres = 1000 + (rand() % 40);
        ESP_LOGI(TAG, "Pres = %.1f hPa", sensor_data.pres);
        xEventGroupSetBits(event_group, PRES_BIT);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void fusion_task(void *p) {
    while (1) {
        EventBits_t bits = xEventGroupWaitBits(event_group, BASIC_ENV,
                                               pdTRUE, pdTRUE, pdMS_TO_TICKS(4000));
        if ((bits & BASIC_ENV) == BASIC_ENV) {
            float comfort = 100 - abs(sensor_data.temp - 25) * 2 - abs(sensor_data.hum - 50) * 0.5;
            ESP_LOGI(TAG, "Basic Fusion: Comfort=%.1f", comfort);
        }
        bits = xEventGroupGetBits(event_group);
        if ((bits & FULL_ENV) == FULL_ENV) {
            float env_index = (sensor_data.temp + sensor_data.hum + sensor_data.pres / 10) / 3.0;
            ESP_LOGI(TAG, "Full Fusion: EnvIndex=%.1f", env_index);
            if (env_index > 200 || env_index < 60)
                xEventGroupSetBits(event_group, ALERT_BIT);
            xEventGroupClearBits(event_group, PRES_BIT);
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void alert_task(void *p) {
    while (1) {
        xEventGroupWaitBits(event_group, ALERT_BIT, pdTRUE, pdTRUE, portMAX_DELAY);
        ESP_LOGW(TAG, "🚨 ALERT: Environmental anomaly!");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void start_exercise2(void) {
    ESP_LOGI(TAG, "===== EXERCISE 2: Sensor Data Fusion =====");
    xTaskCreate(temp_task, "Temp", 2048, NULL, 5, NULL);
    xTaskCreate(hum_task, "Hum", 2048, NULL, 5, NULL);
    xTaskCreate(pres_task, "Pres", 2048, NULL, 5, NULL);
    xTaskCreate(fusion_task, "Fusion", 3072, NULL, 6, NULL);
    xTaskCreate(alert_task, "Alert", 2048, NULL, 4, NULL);
}

// ============================ EXERCISE 3 ============================
#define PHASE1 (BIT0)
#define PHASE2 (BIT1)
#define PHASE3 (BIT2)
#define PHASE4 (BIT3)
#define PHASE5 (BIT4)

void phase_task(void *param) {
    int id = (int)param;
    EventBits_t prereq = (id == 0) ? 0 : (1 << (id - 1));

    if (prereq)
        xEventGroupWaitBits(event_group, prereq, pdFALSE, pdTRUE, portMAX_DELAY);

    ESP_LOGI(TAG, "Phase %d starting...", id + 1);
    vTaskDelay(pdMS_TO_TICKS(1000 + rand() % 2000));
    ESP_LOGI(TAG, "Phase %d complete", id + 1);
    xEventGroupSetBits(event_group, (1 << id));
    vTaskDelete(NULL);
}

void orchestrator_task(void *p) {
    ESP_LOGI(TAG, "Orchestrator monitoring startup...");
    xEventGroupWaitBits(event_group, PHASE5, pdFALSE, pdTRUE, pdMS_TO_TICKS(15000));
    ESP_LOGI(TAG, "🎉 System startup complete!");
    vTaskDelete(NULL);
}

void start_exercise3(void) {
    ESP_LOGI(TAG, "===== EXERCISE 3: Multi-Phase Startup =====");
    for (int i = 0; i < 5; i++)
        xTaskCreate(phase_task, "Phase", 2048, (void *)i, 5 - i, NULL);
    xTaskCreate(orchestrator_task, "Orchestrator", 2048, NULL, 3, NULL);
}

// ============================ EXERCISE 4 ============================
#define T1_READY BIT0
#define T2_READY BIT1
#define T3_READY BIT2
#define T4_READY BIT3
#define ALL_READY (T1_READY | T2_READY | T3_READY | T4_READY)

void worker_task(void *param) {
    int id = (int)param;
    while (1) {
        ESP_LOGI(TAG, "Worker %d: working...", id);
        vTaskDelay(pdMS_TO_TICKS(500 + (rand() % 2000)));
        ESP_LOGI(TAG, "Worker %d: waiting at barrier", id);
        xEventGroupSetBits(event_group, (1 << (id - 1)));
        xEventGroupWaitBits(event_group, ALL_READY, pdTRUE, pdTRUE, portMAX_DELAY);
        ESP_LOGI(TAG, "Worker %d: synchronized, continuing", id);
    }
}

void start_exercise4(void) {
    ESP_LOGI(TAG, "===== EXERCISE 4: Barrier Synchronization =====");
    for (int i = 1; i <= 4; i++)
        xTaskCreate(worker_task, "Worker", 2048, (void *)i, 5, NULL);
}

// ============================ MAIN APP ============================
void app_main(void) {
    srand((unsigned int)xTaskGetTickCount());
    event_group = xEventGroupCreate();
    ESP_LOGI(TAG, "===== FreeRTOS Event Groups Exercises =====");

    int mode = 4;  // 🔧 เปลี่ยนโหมดได้ 1–4 เพื่อรัน Exercise ที่ต้องการ

    switch (mode) {
        case 1: start_exercise1(); break;
        case 2: start_exercise2(); break;
        case 3: start_exercise3(); break;
        case 4: start_exercise4(); break;
        default: ESP_LOGW(TAG, "Invalid mode! Set mode=1–4"); break;
    }
}
