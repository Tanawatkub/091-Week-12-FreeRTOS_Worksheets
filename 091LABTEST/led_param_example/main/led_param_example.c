#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "LED_PARAM_EX4";

typedef struct {
    gpio_num_t pin;
    int delay_ms;
} led_config_t;

void led_task(void *pvParameters)
{
    led_config_t *config = (led_config_t *)pvParameters;
    gpio_set_direction(config->pin, GPIO_MODE_OUTPUT);

    while (1)
    {
        gpio_set_level(config->pin, 1);
        ESP_LOGI(TAG, "LED on GPIO %d ON", config->pin);
        vTaskDelay(pdMS_TO_TICKS(config->delay_ms));

        gpio_set_level(config->pin, 0);
        ESP_LOGI(TAG, "LED on GPIO %d OFF", config->pin);
        vTaskDelay(pdMS_TO_TICKS(config->delay_ms));
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Starting Exercise 4 - Parameter Passing");

    led_config_t led1 = {GPIO_NUM_2, 500};
    led_config_t led2 = {GPIO_NUM_4, 1000};
    led_config_t led3 = {GPIO_NUM_5, 1500};

    xTaskCreate(led_task, "LED1", 2048, &led1, 2, NULL);
    xTaskCreate(led_task, "LED2", 2048, &led2, 2, NULL);
    xTaskCreate(led_task, "LED3", 2048, &led3, 2, NULL);
}
