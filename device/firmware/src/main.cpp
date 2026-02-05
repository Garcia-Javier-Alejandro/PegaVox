/*
 * PegaVox - Phase 2: Core Embedded Firmware
 * ESP32-S3 Thermal Printer Test (C++)
 * 
 * Test: Print "Hello world" when button is pressed
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "ThermalPrinter.hpp"
#include "Button.hpp"

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
    ESP_LOGI(TAG, "PegaVox Firmware - C++ Edition");
    ESP_LOGI(TAG, "Press button to print 'Hello world'");
    
    // Initialize thermal printer
    printer = new ThermalPrinter(UART_NUM_1, PRINTER_TX_PIN, PRINTER_RX_PIN, 9600);
    if (!printer->begin()) {
        ESP_LOGE(TAG, "Failed to initialize printer");
        return;
    }
    
    // Initialize button
    Button* button = new Button(BUTTON_PIN, 50);
    if (!button->begin()) {
        ESP_LOGE(TAG, "Failed to initialize button");
        return;
    }
    
    button->setCallback(onButtonPress);
    
    // Start button task
    xTaskCreate(button_task, "button_task", 2048, button, 10, nullptr);
    
    ESP_LOGI(TAG, "Initialization complete. Ready to print!");
}
