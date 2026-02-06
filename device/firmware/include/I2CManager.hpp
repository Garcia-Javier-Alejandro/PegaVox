/*
 * I2CManager.hpp
 * I2C bus initialization and utilities for OLED displays
 */

#pragma once

#include "driver/i2c.h"
#include "esp_log.h"

class I2CManager {
public:
    I2CManager(gpio_num_t sda_pin = GPIO_NUM_41,
               gpio_num_t scl_pin = GPIO_NUM_42,
               uint32_t freq_hz = 400000);
    ~I2CManager();
    
    bool begin();
    void scan();  // Scan for I2C devices (debugging)
    i2c_port_t get_port() const { return port_; }
    
    // Optional: Bus reset if locked
    void bus_reset();
    
private:
    i2c_port_t port_;
    gpio_num_t sda_pin_;
    gpio_num_t scl_pin_;
    uint32_t freq_hz_;
    bool initialized_;
    
    static constexpr const char* TAG = "I2CManager";
};
