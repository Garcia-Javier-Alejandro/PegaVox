/*
 * Button.cpp
 * Debounced button handler with interrupt support
 */

#include "Button.hpp"
#include "esp_log.h"

static const char* TAG = "Button";

Button::Button(gpio_num_t pin, uint32_t debounce_ms)
    : pin_(pin)
    , debounce_ms_(debounce_ms)
    , callback_(nullptr)
    , event_queue_(nullptr)
    , last_press_time_(0)
{
}

Button::~Button()
{
    if (event_queue_) {
        gpio_isr_handler_remove(pin_);
        vQueueDelete(event_queue_);
    }
}

bool Button::begin()
{
    // Configure GPIO
    gpio_config_t config = {
        .pin_bit_mask = (1ULL << pin_),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE,  // Trigger on falling edge
    };
    
    esp_err_t err = gpio_config(&config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "GPIO config failed: %s", esp_err_to_name(err));
        return false;
    }
    
    // Create event queue
    event_queue_ = xQueueCreate(10, sizeof(uint32_t));
    if (!event_queue_) {
        ESP_LOGE(TAG, "Failed to create event queue");
        return false;
    }
    
    // Install ISR
    gpio_install_isr_service(0);
    gpio_isr_handler_add(pin_, isrHandler, this);
    
    ESP_LOGI(TAG, "Initialized on GPIO %d", pin_);
    return true;
}

void Button::setCallback(Callback callback)
{
    callback_ = callback;
}

void IRAM_ATTR Button::isrHandler(void* arg)
{
    Button* button = static_cast<Button*>(arg);
    uint32_t gpio_num = button->pin_;
    xQueueSendFromISR(button->event_queue_, &gpio_num, nullptr);
}

void Button::task()
{
    uint32_t io_num;
    
    while (xQueueReceive(event_queue_, &io_num, portMAX_DELAY)) {
        TickType_t current_time = xTaskGetTickCount();
        
        // Debounce: ignore if pressed within debounce time
        if ((current_time - last_press_time_) > pdMS_TO_TICKS(debounce_ms_)) {
            // Verify button is actually pressed (LOW with pull-up)
            if (gpio_get_level(pin_) == 0) {
                ESP_LOGI(TAG, "Button pressed");
                
                if (callback_) {
                    callback_();
                }
                
                last_press_time_ = current_time;
            }
        }
    }
}
