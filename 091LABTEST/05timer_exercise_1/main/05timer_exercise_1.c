#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_random.h"

static const char *TAG = "TIMER_EXERCISES";

/* ===========================================================
 * 🧭 Exercise 1: Basic Timer Operations
 * ===========================================================*/

TimerHandle_t one_shot_timer, periodic_timer;

void one_shot_callback(TimerHandle_t xTimer) {
    ESP_LOGI(TAG, "[%lu ms] One-shot timer expired!", xTaskGetTickCount() * portTICK_PERIOD_MS);
}

void periodic_callback(TimerHandle_t xTimer) {
    static int count = 0;
    count++;
    ESP_LOGI(TAG, "[%lu ms] Periodic timer fired #%d", xTaskGetTickCount() * portTICK_PERIOD_MS, count);
}

void control_task(void *pvParameters) {
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));
        ESP_LOGI(TAG, "[CONTROL] Changing timer period to 500 ms");
        xTimerChangePeriod(periodic_timer, pdMS_TO_TICKS(500), 0);

        vTaskDelay(pdMS_TO_TICKS(5000));
        ESP_LOGI(TAG, "[CONTROL] Stopping periodic timer");
        xTimerStop(periodic_timer, 0);

        vTaskDelay(pdMS_TO_TICKS(3000));
        ESP_LOGI(TAG, "[CONTROL] Restarting periodic timer (1 sec)");
        xTimerChangePeriod(periodic_timer, pdMS_TO_TICKS(1000), 0);
        xTimerStart(periodic_timer, 0);
    }
}

/* ===========================================================
 * 💡 Exercise 2: LED Blink Patterns
 * ===========================================================*/

#define LED_PIN GPIO_NUM_2

typedef enum {
    LED_SLOW,
    LED_FAST,
    LED_SOS,
    LED_HEART
} led_pattern_t;

TimerHandle_t led_timer;
led_pattern_t current_pattern = LED_SLOW;
int sos_index = 0;
bool led_state = false;

void led_pattern_callback(TimerHandle_t xTimer) {
    switch (current_pattern) {
        case LED_SLOW:
            led_state = !led_state;
            gpio_set_level(LED_PIN, led_state);
            ESP_LOGI(TAG, "LED SLOW: %s", led_state ? "ON" : "OFF");
            xTimerChangePeriod(led_timer, pdMS_TO_TICKS(1000), 0);
            break;

        case LED_FAST:
            led_state = !led_state;
            gpio_set_level(LED_PIN, led_state);
            ESP_LOGI(TAG, "LED FAST: %s", led_state ? "ON" : "OFF");
            xTimerChangePeriod(led_timer, pdMS_TO_TICKS(200), 0);
            break;

        case LED_SOS: {
            static const int sos_pattern[] = {1,0,1,0,1,0, 1,1,1,0,1,1,1,0,1,1,1,0, 1,0,1,0,1,0,0};
            gpio_set_level(LED_PIN, sos_pattern[sos_index]);
            ESP_LOGI(TAG, "LED SOS: %s", sos_pattern[sos_index] ? "ON" : "OFF");
            sos_index = (sos_index + 1) % (sizeof(sos_pattern)/sizeof(sos_pattern[0]));
            xTimerChangePeriod(led_timer, pdMS_TO_TICKS(200), 0);
            break;
        }

        case LED_HEART:
            led_state = !led_state;
            gpio_set_level(LED_PIN, led_state);
            ESP_LOGI(TAG, "LED HEARTBEAT: %s", led_state ? "ON" : "OFF");
            xTimerChangePeriod(led_timer, pdMS_TO_TICKS(700), 0);
            break;
    }
}

void led_control_task(void *pvParameters) {
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000)); // change pattern every 10s
        current_pattern = (current_pattern + 1) % 4;
        ESP_LOGI(TAG, "🔄 Switching LED pattern to %d", current_pattern);
        xTimerReset(led_timer, 0);
    }
}

/* ===========================================================
 * 🌡️ Exercise 3: Adaptive Sensor Sampling
 * ===========================================================*/

typedef struct {
    float temperature;
    float humidity;
    uint32_t tick;
} sensor_data_t;

QueueHandle_t sensor_queue;
TimerHandle_t sensor_timer;

void sensor_timer_callback(TimerHandle_t xTimer) {
    sensor_data_t data;
    data.temperature = 20 + (esp_random() % 3000) / 100.0;
    data.humidity = 40 + (esp_random() % 6000) / 100.0;
    data.tick = xTaskGetTickCount();

    xQueueSend(sensor_queue, &data, 0);

    TickType_t next_period;
    if (data.temperature > 40.0)
        next_period = pdMS_TO_TICKS(500);
    else if (data.temperature > 30.0)
        next_period = pdMS_TO_TICKS(1000);
    else
        next_period = pdMS_TO_TICKS(2000);

    xTimerChangePeriod(sensor_timer, next_period, 0);
}

void sensor_task(void *pvParameters) {
    sensor_data_t data;
    while (1) {
        if (xQueueReceive(sensor_queue, &data, portMAX_DELAY)) {
            ESP_LOGI(TAG, "Sensor: %.1f°C, %.1f%%RH (tick %lu)", data.temperature, data.humidity, data.tick);
            if (data.temperature > 45.0) ESP_LOGW(TAG, "🔥 Overheat detected!");
            if (data.humidity > 90.0) ESP_LOGW(TAG, "💧 High humidity detected!");
        }
    }
}

/* ===========================================================
 * 🧠 Exercise 4: System Health Monitor
 * ===========================================================*/

TimerHandle_t watchdog_timer, heartbeat_timer;

void watchdog_callback(TimerHandle_t xTimer) {
    ESP_LOGE(TAG, "⚠️ WATCHDOG TIMEOUT - system not responding");
    xTimerReset(watchdog_timer, 0);
}

void heartbeat_callback(TimerHandle_t xTimer) {
    static int beat = 0;
    beat++;
    ESP_LOGI(TAG, "❤️ System heartbeat #%d", beat);
    xTimerReset(watchdog_timer, 0);
}

void app_main(void) {
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    /* === Exercise 1 === */
    one_shot_timer = xTimerCreate("OneShot", pdMS_TO_TICKS(3000), pdFALSE, 0, one_shot_callback);
    periodic_timer = xTimerCreate("Periodic", pdMS_TO_TICKS(1000), pdTRUE, 0, periodic_callback);
    xTimerStart(one_shot_timer, 0);
    xTimerStart(periodic_timer, 0);
    xTaskCreate(control_task, "Control", 2048, NULL, 3, NULL);

    /* === Exercise 2 === */
    led_timer = xTimerCreate("LEDTimer", pdMS_TO_TICKS(1000), pdTRUE, 0, led_pattern_callback);
    xTimerStart(led_timer, 0);
    xTaskCreate(led_control_task, "LEDControl", 2048, NULL, 2, NULL);

    /* === Exercise 3 === */
    sensor_queue = xQueueCreate(10, sizeof(sensor_data_t));
    sensor_timer = xTimerCreate("Sensor", pdMS_TO_TICKS(1000), pdTRUE, 0, sensor_timer_callback);
    xTimerStart(sensor_timer, 0);
    xTaskCreate(sensor_task, "Sensor", 2048, NULL, 4, NULL);

    /* === Exercise 4 === */
    watchdog_timer = xTimerCreate("Watchdog", pdMS_TO_TICKS(5000), pdFALSE, 0, watchdog_callback);
    heartbeat_timer = xTimerCreate("Heartbeat", pdMS_TO_TICKS(1000), pdTRUE, 0, heartbeat_callback);
    xTimerStart(watchdog_timer, 0);
    xTimerStart(heartbeat_timer, 0);

    ESP_LOGI(TAG, "✅ All timer exercises started!");
}
