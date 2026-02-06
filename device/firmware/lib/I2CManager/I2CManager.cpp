/*
 * I2CManager.cpp
 * I2C bus initialization and utilities for OLED displays
 */

#include "I2CManager.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cstdio>

I2CManager::I2CManager(gpio_num_t sda_pin, gpio_num_t scl_pin, uint32_t freq_hz)
    : port_(I2C_NUM_0)
    , sda_pin_(sda_pin)
    , scl_pin_(scl_pin)
    , freq_hz_(freq_hz)
    , initialized_(false)
{
}

I2CManager::~I2CManager()
{
    if (initialized_) {
        i2c_driver_delete(port_);
    }
}

bool I2CManager::begin()
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = sda_pin_,
        .scl_io_num = scl_pin_,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,   // Enable if no external pull-ups
        .scl_pullup_en = GPIO_PULLUP_ENABLE,   // Enable if no external pull-ups
        .master = {
            .clk_speed = freq_hz_,
        },
        .clk_flags = 0,
    };
    
    esp_err_t err = i2c_param_config(port_, &conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2C param config failed: %s", esp_err_to_name(err));
        return false;
    }
    
    err = i2c_driver_install(port_, conf.mode, 0, 0, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2C driver install failed: %s", esp_err_to_name(err));
        return false;
    }
    
    initialized_ = true;
    ESP_LOGI(TAG, "I2C initialized: SDA=%d, SCL=%d, Freq=%d Hz",
             sda_pin_, scl_pin_, freq_hz_);
    
    return true;
}

void I2CManager::scan()
{
    esp_err_t ret;
    printf("\nI2C Scanner Results:\n");
    printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\r\n");
    
    for (int i = 0; i < 128; i += 16) {
        printf("%02x: ", i);
        for (int j = 0; j < 16; j++) {
            uint8_t address = i + j;
            i2c_cmd_handle_t cmd = i2c_cmd_link_create();
            i2c_master_start(cmd);
            i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, true);
            i2c_master_stop(cmd);
            ret = i2c_master_cmd_begin(port_, cmd, 50 / portTICK_PERIOD_MS);
            i2c_cmd_link_delete(cmd);
            
            if (ret == ESP_OK) {
                printf("%02x ", address);
            } else {
                printf("-- ");
            }
        }
        printf("\r\n");
    }
}

void I2CManager::bus_reset()
{
    ESP_LOGI(TAG, "Attempting I2C bus reset...");
    
    // Configure SCL as open-drain output
    gpio_config_t scl_config = {
        .pin_bit_mask = (1ULL << scl_pin_),
        .mode = GPIO_MODE_OUTPUT_OD,
        .pull_up_en = GPIO_PULLUP_DISABLE,
    };
    gpio_config(&scl_config);
    
    // Clock SCL 9 times to release any stuck devices
    for (int i = 0; i < 9; i++) {
        gpio_set_level(scl_pin_, 0);
        vTaskDelay(pdMS_TO_TICKS(5));
        gpio_set_level(scl_pin_, 1);
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    
    ESP_LOGI(TAG, "Bus reset complete. Reinitialize I2C driver.");
    i2c_driver_delete(port_);
    vTaskDelay(pdMS_TO_TICKS(100));
    begin();
}
