/*
 * LED.hpp
 * Simple LED control
 */

#pragma once

#include "driver/gpio.h"

class LED {
public:
    LED(gpio_num_t pin);
    ~LED();
    
    bool begin();
    void on();
    void off();
    void toggle();
    void blink(int times, int delay_ms = 100);
    
private:
    gpio_num_t pin_;
    bool state_;
    bool initialized_;
};
