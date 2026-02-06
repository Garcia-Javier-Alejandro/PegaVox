# PegaVox ESP32-S3 Firmware

## Overview

This firmware implements the embedded controller for the PegaVox voice-driven sticker printer.

**Language**: C++ (ESP-IDF with C++ support via PlatformIO)  
**Build System**: PlatformIO  
**Current Phase**: Phase 2 - Core Embedded Firmware  
**Test Status**: Button → Print "Hello world" via UART thermal printer

## Architecture

The firmware uses a class-based architecture for maintainability and future expansion:

- **`ThermalPrinter`**: ESC/POS thermal printer driver (UART)
- **`Button`**: Debounced button handler with interrupt support
- **`main.cpp`**: Application entry point and initialization

**Future Components (Not Yet Implemented):**
- **OLED Display**: SSD1327 I2C driver (GPIO 41/42) for status display
- **I2S Microphone**: INMP441 audio capture (GPIO 4-6) for voice input
- **Wi-Fi Manager**: Network connectivity and provisioning

## Hardware Requirements

- ESP32-S3 Dev Board
- Thermal printer with TTL UART interface (e.g., Cashino, CSN-A2)
- 30mm arcade push button
- SSD1327 OLED 128×128 display (I2C) – Phase 3+
- INMP441 I2S MEMS microphone – Phase 4+

**For detailed electrical configuration, pull-up requirements, and firmware setup**, see [BUTTON_AND_I2C_SETUP.md](BUTTON_AND_I2C_SETUP.md)

## Pin Configuration

See [../../docs/pinout.md](../../docs/pinout.md) for complete pin mapping.

**Current Test Uses (Phase 2):**
- GPIO 12: Button input (active-low, internal pull-up)
- GPIO 17: UART TX (Printer)
- GPIO 18: UART RX (Printer)

**Future Expansion (Phase 3+):**
- GPIO 41: I2C SDA (OLED display)
- GPIO 42: I2C SCL (OLED display)
- GPIO 4-6: I2S bus (microphone audio)

## Building and Flashing

### Prerequisites

1. Install VS Code
2. Install **PlatformIO IDE** extension (platformio.ideaspeed)
3. Connect ESP32-S3 board via USB

### Build

**Option 1: Via PlatformIO UI**
- Click PlatformIO icon (left sidebar)
- Click "Build" under the env

**Option 2: Via Terminal**
```bash
cd device/firmware
pio run
```

### Flash

**Option 1: Via PlatformIO UI**
- Connect ESP32-S3 via USB
- Click "Upload" in PlatformIO sidebar

**Option 2: Via Terminal**
```bash
pio run --target upload
```

### Monitor

**Option 1: Via PlatformIO UI**
- Click "Monitor" in PlatformIO sidebar

**Option 2: Via Terminal**
```bash
pio device monitor  # or
pio run --target monitor
```

To exit monitor: `Ctrl+C`

## Testing

1. Flash firmware to ESP32-S3
2. Connect thermal printer to UART pins (TX=17, RX=18)
3. Ensure printer and ESP32 share common ground
4. Power on printer (dedicated 5V supply recommended)
5. Press button on GPIO 12
6. Printer should output "Hello world", "PegaVox Test Print", and "C++ Edition"
7. Check serial monitor for confirmation logs

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

If your printer uses a different baud rate, modify `UART_BAUD_RATE` in [src/main.cpp](src/main.cpp).

## Troubleshooting

### Printer Not Responding

1. Check wiring: ESP32 TX → Printer RX, ESP32 RX → Printer TX
2. Verify common ground connection
3. Check printer power supply (5V with sufficient current, typically 2A+)
4. Try different baud rates (9600, 19200, 115200)
5. Monitor UART output: `idf.py monitor`

### Button Not Working

1. Check button wiring to GPIO 12 → GND
2. Verify button reads LOW when pressed using multimeter
3. Check debounce logs in serial monitor
4. Internal pull-up is enabled; no external resistor needed
5. See [BUTTON_AND_I2C_SETUP.md](BUTTON_AND_I2C_SETUP.md) for detailed troubleshooting

### Serial Monitor

Use the PlatformIO monitor to see debug logs:
```
PegaVox Firmware - C++ Edition
Press button to print 'Hello world'
Initialization complete. Ready to print!
```

When button is pressed:
```
Button pressed
Button pressed! Printing...
Print complete!
```

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

### Future: OLED Display Driver (Phase 3+)

```cpp
// Planned structure:
// OLED oled(GPIO_NUM_41, GPIO_NUM_42);  // I2C SDA, SCL
// oled.begin();
// oled.println("Ready to print");
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
├── platformio.ini          # PlatformIO project configuration
├── README.md               # This file
├── src/
│   └── main.cpp            # Application entry point
├── include/
│   ├── ThermalPrinter.hpp
│   ├── Button.hpp
│   └── LED.hpp
├── lib/
│   ├── ThermalPrinter/
│   │   ├── ThermalPrinter.cpp
│   │   └── library.json
│   ├── Button/
│   │   ├── Button.cpp
│   │   └── library.json
│   └── LED/
│       ├── LED.cpp
│       └── library.json
├── .gitignore
└── secrets_example.hpp     # Template for secrets
```

**PlatformIO Structure Explained:**

- **`platformio.ini`**: Project configuration
  - Specifies board, platform, framework, compiler flags
  - Defines serial monitor speed
  - Configures source/include/library directories

- **`src/`**: Source files (must be `src/` for PlatformIO)
  - `main.cpp`: Entry point with `app_main()`

- **`include/`**: Headers included by `src/`
  - Class interfaces (.hpp files)
  - Automatically added to compiler include path

- **`lib/`**: Internal libraries
  - Each folder is a separate library component
  - `library.json` declares library metadata
  - Automatically compiled and linked by PlatformIO
## License

See project root LICENSE file.
