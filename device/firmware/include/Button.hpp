/*
 * Button.hpp
 * Debounced button handler with interrupt support
 */

#pragma once

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <functional>

class Button {
public:
    using Callback = std::function<void()>;
    
    Button(gpio_num_t pin, uint32_t debounce_ms = 50);
    ~Button();
    
    bool begin();
    void setCallback(Callback callback);
    void task();  // Call from FreeRTOS task
    
private:
    gpio_num_t pin_;
    uint32_t debounce_ms_;
    Callback callback_;
    QueueHandle_t event_queue_;
    TickType_t last_press_time_;
    
    static void IRAM_ATTR isrHandler(void* arg);
};
