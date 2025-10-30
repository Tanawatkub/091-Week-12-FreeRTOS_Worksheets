#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include <stdlib.h>

static const char *TAG = "QUEUE_EX4";

typedef struct {
    float value;
    uint32_t timestamp;
} sensor_data_t;

QueueHandle_t raw_queue;
QueueHandle_t processed_queue;
QueueHandle_t alert_queue;

void sensor_reader_task(void *pv)
{
    sensor_data_t data;
    while (1)
    {
        data.value = 20 + (rand() % 200) / 10.0;
        data.timestamp = xTaskGetTickCount();
        xQueueSend(raw_queue, &data, pdMS_TO_TICKS(100));
        ESP_LOGI(TAG, "Read sensor: %.1f", data.value);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void data_processor_task(void *pv)
{
    sensor_data_t in, out;
    while (1)
    {
        if (xQueueReceive(raw_queue, &in, portMAX_DELAY))
        {
            out.value = in.value * 0.98; // filter
            out.timestamp = in.timestamp;
            xQueueSend(processed_queue, &out, pdMS_TO_TICKS(100));
            ESP_LOGI(TAG, "Processed data: %.1f", out.value);
            if (out.value > 35.0)
                xQueueSend(alert_queue, &out, 0);
        }
    }
}

void alert_handler_task(void *pv)
{
    sensor_data_t alert;
    while (1)
    {
        if (xQueueReceive(alert_queue, &alert, portMAX_DELAY))
        {
            ESP_LOGW(TAG, "ALERT! High temperature: %.1f °C", alert.value);
        }
    }
}

void logger_task(void *pv)
{
    sensor_data_t data;
    while (1)
    {
        if (xQueueReceive(processed_queue, &data, pdMS_TO_TICKS(5000)))
        {
            ESP_LOGI(TAG, "Logger: %.1f°C @%d", data.value, data.timestamp);
        }
    }
}

void app_main(void)
{
    raw_queue = xQueueCreate(10, sizeof(sensor_data_t));
    processed_queue = xQueueCreate(10, sizeof(sensor_data_t));
    alert_queue = xQueueCreate(5, sizeof(sensor_data_t));

    if (!raw_queue || !processed_queue || !alert_queue)
    {
        ESP_LOGE(TAG, "Queue creation failed!");
        return;
    }

    xTaskCreate(sensor_reader_task, "Sensor", 2048, NULL, 3, NULL);
    xTaskCreate(data_processor_task, "Processor", 3072, NULL, 3, NULL);
    xTaskCreate(alert_handler_task, "Alert", 2048, NULL, 4, NULL);
    xTaskCreate(logger_task, "Logger", 2048, NULL, 2, NULL);
}
