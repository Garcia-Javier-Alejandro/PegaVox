/*
 * LED.cpp
 * Simple LED control
 */

#include "LED.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "LED";

LED::LED(gpio_num_t pin)
    : pin_(pin)
    , state_(false)
    , initialized_(false)
{
}

LED::~LED()
{
    if (initialized_) {
        off();
    }
}

bool LED::begin()
{
    gpio_config_t config = {
        .pin_bit_mask = (1ULL << pin_),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    
    esp_err_t err = gpio_config(&config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "GPIO config failed: %s", esp_err_to_name(err));
        return false;
    }
    
    off();
    initialized_ = true;
    ESP_LOGI(TAG, "Initialized on GPIO %d", pin_);
    return true;
}

void LED::on()
{
    gpio_set_level(pin_, 1);
    state_ = true;
}

void LED::off()
{
    gpio_set_level(pin_, 0);
    state_ = false;
}

void LED::toggle()
{
    state_ ? off() : on();
}

void LED::blink(int times, int delay_ms)
{
    for (int i = 0; i < times; i++) {
        on();
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
        off();
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
    }
}
