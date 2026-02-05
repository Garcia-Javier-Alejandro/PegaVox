/*
 * ThermalPrinter.cpp
 * Driver for ESC/POS thermal printers over UART
 */

#include "ThermalPrinter.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cstring>

ThermalPrinter::ThermalPrinter(uart_port_t port, int tx_pin, int rx_pin, int baud_rate)
    : uart_port_(port)
    , tx_pin_(tx_pin)
    , rx_pin_(rx_pin)
    , baud_rate_(baud_rate)
    , initialized_(false)
{
}

ThermalPrinter::~ThermalPrinter()
{
    if (initialized_) {
        uart_driver_delete(uart_port_);
    }
}

bool ThermalPrinter::begin()
{
    uart_config_t uart_config = {
        .baud_rate = baud_rate_,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    
    esp_err_t err = uart_driver_install(uart_port_, UART_BUF_SIZE * 2, 0, 0, nullptr, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "UART driver install failed: %s", esp_err_to_name(err));
        return false;
    }
    
    err = uart_param_config(uart_port_, &uart_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "UART param config failed: %s", esp_err_to_name(err));
        uart_driver_delete(uart_port_);
        return false;
    }
    
    err = uart_set_pin(uart_port_, tx_pin_, rx_pin_, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "UART set pin failed: %s", esp_err_to_name(err));
        uart_driver_delete(uart_port_);
        return false;
    }
    
    initialized_ = true;
    ESP_LOGI(TAG, "Initialized: TX=%d, RX=%d, Baud=%d", tx_pin_, rx_pin_, baud_rate_);
    
    // Initialize printer
    reset();
    
    return true;
}

void ThermalPrinter::reset()
{
    // ESC @ - Initialize printer
    const uint8_t init_cmd[] = {0x1B, 0x40};
    sendCommand(init_cmd, sizeof(init_cmd));
    vTaskDelay(pdMS_TO_TICKS(100));
}

void ThermalPrinter::sendCommand(const uint8_t* cmd, size_t len)
{
    if (!initialized_) {
        ESP_LOGW(TAG, "Printer not initialized");
        return;
    }
    uart_write_bytes(uart_port_, (const char*)cmd, len);
}

void ThermalPrinter::sendText(const char* text)
{
    if (!initialized_) {
        ESP_LOGW(TAG, "Printer not initialized");
        return;
    }
    uart_write_bytes(uart_port_, text, strlen(text));
}

void ThermalPrinter::printText(const char* text)
{
    sendText(text);
}

void ThermalPrinter::printLine(const char* text)
{
    sendText(text);
    sendText("\n");
}

void ThermalPrinter::feedLines(uint8_t lines)
{
    // ESC d n - Feed n lines
    const uint8_t feed_cmd[] = {0x1B, 0x64, lines};
    sendCommand(feed_cmd, sizeof(feed_cmd));
    vTaskDelay(pdMS_TO_TICKS(100));
}

void ThermalPrinter::cutPaper()
{
    // GS V m - Partial cut (if supported)
    const uint8_t cut_cmd[] = {0x1D, 0x56, 0x01};
    sendCommand(cut_cmd, sizeof(cut_cmd));
    vTaskDelay(pdMS_TO_TICKS(500));
}
