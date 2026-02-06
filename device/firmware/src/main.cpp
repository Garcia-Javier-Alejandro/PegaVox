/*
 * PegaVox - Phase 2: Core Embedded Firmware
 * ESP32-S3 Thermal Printer + Button + I2C Test (C++)
 * 
 * Features:
 * - Print "Hello world" when button (GPIO 12) is pressed
 * - I2C bus initialized for future OLED display (GPIO 41/42)
 * - Button debouncing (50ms)
 * - I2C device scanner for verification
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "ThermalPrinter.hpp"
#include "Button.hpp"
#include "I2CManager.hpp"

// Pin definitions
#define PRINTER_TX_PIN      GPIO_NUM_17
#define PRINTER_RX_PIN      GPIO_NUM_18
#define MIC_BCLK_PIN        GPIO_NUM_4
#define MIC_WS_PIN          GPIO_NUM_5
#define MIC_DATA_PIN        GPIO_NUM_6
#define OLED_SDA_PIN        GPIO_NUM_41
#define OLED_SCL_PIN        GPIO_NUM_42
#define BUTTON_PIN          GPIO_NUM_12

static const char *TAG = "PegaVox";

// Global objects
static ThermalPrinter* printer = nullptr;
static I2CManager* i2c_manager = nullptr;

// Button press handler
void onButtonPress()
{
    ESP_LOGI(TAG, "Button pressed! Printing...");
    
    printer->reset();
    printer->printLine("Hello world");
    printer->printLine("PegaVox Test Print");
    printer->printLine("C++ Edition");
    printer->feedLines(3);
    printer->cutPaper();
    
    ESP_LOGI(TAG, "Print complete!");
}

// Button task wrapper
void button_task(void* arg)
{
    Button* button = static_cast<Button*>(arg);
    button->task();
}

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "===========================================");
    ESP_LOGI(TAG, "PegaVox Firmware - C++ Edition");
    ESP_LOGI(TAG, "Phase 2: Printer + Button + I2C");
    ESP_LOGI(TAG, "===========================================");
    
    // ===== Initialize I2C Bus =====
    ESP_LOGI(TAG, "Initializing I2C bus for OLED display...");
    i2c_manager = new I2CManager(OLED_SDA_PIN, OLED_SCL_PIN, 400000);
    if (!i2c_manager->begin()) {
        ESP_LOGE(TAG, "Failed to initialize I2C bus");
        // Continue anyway—printer will still work
    } else {
        ESP_LOGI(TAG, "I2C bus initialized successfully");
        
        // Scan for I2C devices (OLED should be at 0x3C)
        vTaskDelay(pdMS_TO_TICKS(100));
        i2c_manager->scan();
    }
    
    // ===== Initialize Thermal Printer =====
    ESP_LOGI(TAG, "Initializing thermal printer (UART)...");
    printer = new ThermalPrinter(UART_NUM_1, PRINTER_TX_PIN, PRINTER_RX_PIN, 9600);
    if (!printer->begin()) {
        ESP_LOGE(TAG, "Failed to initialize printer");
        return;
    }
    
    // ===== Initialize Button =====
    ESP_LOGI(TAG, "Initializing button (GPIO %d)...", BUTTON_PIN);
    Button* button = new Button(BUTTON_PIN, 50);
    if (!button->begin()) {
        ESP_LOGE(TAG, "Failed to initialize button");
        return;
    }
    
    // Set button callback
    button->setCallback(onButtonPress);
    
    // ===== Start Button Task =====
    xTaskCreate(button_task, "button_task", 2048, button, 10, nullptr);
    
    // ===== Initialization Complete =====
    ESP_LOGI(TAG, "===========================================");
    ESP_LOGI(TAG, "Initialization complete!");
    ESP_LOGI(TAG, "Ready to accept button presses...");
    ESP_LOGI(TAG, "===========================================");
    
    // Keep app_main alive (FreeRTOS scheduler handles everything else)
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
