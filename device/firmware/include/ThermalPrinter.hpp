/*
 * ThermalPrinter.hpp
 * Driver for ESC/POS thermal printers over UART
 */

#pragma once

#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"

class ThermalPrinter {
public:
    ThermalPrinter(uart_port_t port, int tx_pin, int rx_pin, int baud_rate = 9600);
    ~ThermalPrinter();
    
    bool begin();
    void printText(const char* text);
    void printLine(const char* text);
    void feedLines(uint8_t lines);
    void cutPaper();
    void reset();
    
private:
    uart_port_t uart_port_;
    int tx_pin_;
    int rx_pin_;
    int baud_rate_;
    bool initialized_;
    
    static constexpr size_t UART_BUF_SIZE = 1024;
    static constexpr char* TAG = "ThermalPrinter";
    
    void sendCommand(const uint8_t* cmd, size_t len);
    void sendText(const char* text);
};
