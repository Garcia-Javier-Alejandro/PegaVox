# PegaVox ESP32-S3 Firmware

## Overview

This firmware implements the embedded controller for the PegaVox voice-driven sticker printer.

**Language**: C++ (ESP-IDF with C++ support)  
**Current Phase**: Phase 2 - Core Embedded Firmware  
**Test Status**: Button → Print "Hello world" via UART thermal printer

## Architecture

The firmware uses a class-based architecture for maintainability and future expansion:

- **`ThermalPrinter`**: ESC/POS thermal printer driver (UART)
- **`Button`**: Debounced button handler with interrupt support
- **`LED`**: Status indicator control
- **`main.cpp`**: Application entry point and initialization

## Hardware Requirements

- ESP32-S3 Dev Board
- Thermal printer with TTL UART interface (e.g., Cashino, CSN-A2)
- 30mm arcade push button
- LED indicator
- INMP441 I2S MEMS microphone (future)
- SSD1327 OLED display (future)

## Pin Configuration

See [../../docs/pinout.md](../../docs/pinout.md) for complete pin mapping.

**Current Test Uses:**
- GPIO 12: Button input (pull-up enabled)
- GPIO 15: LED indicator
- GPIO 17: UART TX → Printer RX
- GPIO 18: UART RX → Printer TX

## Building and Flashing

### Prerequisites

1. Install ESP-IDF v5.0 or later:
   ```bash
   # Follow official guide: https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/get-started/
   ```

2. Set up ESP-IDF environment:
   ```bash
   # Linux/macOS
   . $HOME/esp/esp-idf/export.sh
   
   # Windows (Command Prompt)
   %userprofile%\esp\esp-idf\export.bat
   
   # Windows (PowerShell)
   .$HOME\esp\esp-idf\export.ps1
   ```

### Build

```bash
cd device/firmware
idf.py set-target esp32s3
idf.py build
```

### Flash

```bash
idf.py -p COM3 flash monitor
```

Replace `COM3` with your serial port (Linux: `/dev/ttyUSB0`, macOS: `/dev/cu.usbserial-*`)

### Monitor Only

```bash
idf.py -p COM3 monitor
```

To exit monitor: `Ctrl+]`

## Testing

1. Flash firmware to ESP32-S3
2. Connect thermal printer to UART pins (TX=17, RX=18)
3. Ensure printer and ESP32 share common ground
4. Power on printer (dedicated 5V supply recommended)
5. Press button on GPIO 12
6. LED on GPIO 15 should light during printing
7. Printer should output "Hello world" and "PegaVox Test Print"

## Thermal Printer Configuration

**Default Settings:**
- Baud rate: 9600
- Data bits: 8
- Parity: None
- Stop bits: 1
- Flow control: None

**Common Thermal Printer Baud Rates:**
- 9600 (default in this firmware)
- 19200
- 38400
- 115200

If your printer uses a different baud rate, modify `UART_BAUD_RATE` in `main/main.c`.

## Troubleshooting

### Printer Not Responding

1. Check wiring: ESP32 TX → Printer RX, ESP32 RX → Printer TX
2. Verify common ground connection
3. Check printer power supply (5V with sufficient current, typically 2A+)
4. Try different baud rates (9600, 19200, 115200)
5. Monitor UART output: `idf.py monitor`

### Button Not Working

1. Check button wiring to GPIO 12
2. Verify button connects GPIO 12 to GND when pressed
3. Check debounce logs in serial monitor
4. Internal pull-up is enabled, no external resistor needed

### LED Not Lighting

1. Verify LED anode → GPIO 15, cathode → GND (via current-limiting resistor)
2. Use 220Ω-1kΩ resistor in series with LED
3. Check if LED lights during startup blink sequence (3 blinks)

### Print Quality Issues

1. Check printer paper is thermal paper (heat-sensitive coating)
2. Ensure print head is clean
3. Adjust darkness/heat settings if your printer supports it
4. Some printers require ESC/POS configuration commands

## Code Structure

### ThermalPrinter Class

```cpp
ThermalPrinter printer(UART_NUM_1, TX_PIN, RX_PIN, 9600);
printer.begin();
printer.printLine("Hello world");
printer.feedLines(3);
printer.cutPaper();
```

### Button Class

```cpp
Button button(GPIO_NUM_12, 50);  // 50ms debounce
button.begin();
button.setCallback([]() {
    // Handle button press
});
xTaskCreate(buttonTask, "button", 2048, &button, 10, nullptr);
```

### LED Class

```cpp
LED led(GPIO_NUM_15);
led.begin();
led.on();
led.blink(3, 100);  // Blink 3 times, 100ms each
```

## Next Steps (Phase 2 Continuation)

- [ ] Add HTTPS client for backend communication
- [ ] Implement device authentication
- [ ] Add OTA firmware update support
- [ ] Implement OLED display driver (I2C)
- [ ] Add I2S microphone capture
- [ ] Implement UI state machine
- [ ] Add Wi-Fi provisioning via BLE

## Project Structure

```
firmware/
├── CMakeLists.txt              # Root CMake configuration
├── sdkconfig.defaults          # Default ESP-IDF settings
├── README.md                   # This file
├── secrets_example.h           # Template for Wi-Fi credentials (future)
├── secrets.h                   # Actual credentials (gitignored, future)
└── main/
    ├── CMakeLists.txt          # Main component CMake
    ├── main.cpp                # Application entry point
    ├── ThermalPrinter.hpp      # Printer driver header
    ├── ThermalPrinter.cpp      # Printer driver implementation
    ├── Button.hpp              # Button handler header
    ├── Button.cpp              # Button handler implementation
    ├── LED.hpp                 # LED control header
    └── LED.cpp                 # LED control implementation
```

## Why Two CMakeLists.txt Files?

ESP-IDF uses a component-based build system:

1. **Root CMakeLists.txt** (`firmware/CMakeLists.txt`):
   - Defines the project name
   - Includes ESP-IDF build system
   - Required for all ESP-IDF projects

2. **Component CMakeLists.txt** (`firmware/main/CMakeLists.txt`):
   - Registers source files (`.cpp`, `.c`)
   - Specifies include directories
   - Each component (folder) needs one
   - `main/` is the primary application component

This structure allows you to add more components later (e.g., `components/wifi_manager/`, `components/audio_capture/`).

## License

See project root LICENSE file.
