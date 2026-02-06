# PegaVox ESP32-S3 Pinout

## Pin Mapping

| Function      | GPIO | Notes                                    |
|---------------|------|------------------------------------------|
| **Printer**   |      |                                          |
| Printer TX    | 17   | ESP32 TX → Printer RX                    |
| Printer RX    | 18   | ESP32 RX → Printer TX                    |
| **Microphone**|      |                                          |
| Mic BCLK      | 4    | I2S bit clock (INMP441)                  |
| Mic WS        | 5    | I2S word select / LRCLK                  |
| Mic DATA      | 6    | I2S serial data input                    |
| **Display**   |      |                                          |
| OLED SDA      | 41   | I2C data (SSD1327)                       |
| OLED SCL      | 42   | I2C clock (SSD1327)                      |
| **User I/O**  |      |                                          |
| Button        | 12   | Active-low, internal pull-up enabled     |
| **Available** |      |                                          |
| Spare         | 13   | Reserved for future expansion            |
| Spare         | 14   | Reserved for future expansion            |
| Spare         | 15   | Reserved for future expansion            |
| Spare         | 16   | Reserved for future expansion            |

## Pin Configuration Details

### UART (Thermal Printer)

- **Protocol**: UART (TTL level, 3.3V)
- **Default Baud**: 9600 (configurable: 9600, 19200, 38400, 115200)
- **Data Format**: 8N1 (8 data bits, no parity, 1 stop bit)
- **Flow Control**: None
- **Notes**: 
  - ESP32 TX (GPIO 17) connects to printer RX
  - ESP32 RX (GPIO 18) connects to printer TX
  - Common ground required between ESP32 and printer
  - Printer may require dedicated 5V power supply (2A+)

### I2S (INMP441 Microphone)

- **Protocol**: I2S (Inter-IC Sound)
- **Sample Rate**: 16 kHz (configurable)
- **Bit Depth**: 16-bit or 32-bit
- **Channel**: Mono (left channel)
- **Notes**:
  - BCLK (bit clock): GPIO 4
  - WS/LRCLK (word select): GPIO 5
  - DIN (serial data): GPIO 6
  - INMP441 L/R pin: tie to GND for left channel, VCC for right channel

### I2C (SSD1327 OLED Display)

- **Protocol**: I2C (open-drain bus, requires pull-ups to 3.3V)
- **Address**: 0x3C (typical for SSD1327)
- **Speed**: 400 kHz (Fast Mode, standard for OLED)
- **Pins**:
  - SDA (data): GPIO 41
  - SCL (clock): GPIO 42
- **Power**: 3.3V supply
- **Pull-ups**: 
  - **Status**: Most SSD1327 breakouts include 4.7kΩ pull-ups on PCB
  - **To verify**: Check for resistor markings on breakout or use multimeter (should read ~4.7kΩ to 3.3V)
  - **If missing**: Add external 4.7kΩ resistors: `SDA → 3.3V` and `SCL → 3.3V`
- **Status**: Planned for Phase 3 (OLED display driver not yet implemented)
- **See Also**: [BUTTON_AND_I2C_SETUP.md](../device/firmware/BUTTON_AND_I2C_SETUP.md) for detailed hardware and firmware configuration

### Button (User Input)

- **Type**: Normally-open momentary switch
- **Pin**: GPIO 12
- **Configuration**: Input with internal pull-up
- **Trigger**: Falling edge (active-low)
- **Debounce**: 50ms in software
- **Recommended**: 30mm arcade button for child-friendly use

### LED (Status Indicator)

- **Pin**: GPIO 15
- **Configuration**: Output (active-high)
- **Current**: 20mA max
- **Recommended**: Use 220Ω-1kΩ current-limiting resistor
- **Connection**: Anode → Resistor → GPIO 15, Cathode → GND

## Power Considerations

### ESP32-S3

- **Operating Voltage**: 3.3V
- **Current Draw**: 
  - Normal operation: ~80mA
  - Wi-Fi TX peak: ~240mA
  - Deep sleep: <10µA

### Thermal Printer

- **Operating Voltage**: 5V (typical)
- **Current Draw**:
  - Idle: ~50mA
  - Printing (peak): 1.5-2.5A
- **Recommendation**: Dedicated 5V supply with 2A+ capacity
- **Critical**: Printer supply and ESP32 supply must share common ground

### Power Supply Strategy

1. **Option A**: Single 5V supply
   - 5V supply → Buck regulator → 3.3V for ESP32
   - 5V supply → Thermal printer directly

2. **Option B**: Dual supplies
   - 3.3V LDO → ESP32 and peripherals
   - 5V supply → Thermal printer only
   - **Must connect grounds together**

## Electrical Notes

### Signal Levels

- **ESP32-S3**: 3.3V logic
- **INMP441**: 3.3V compatible
- **SSD1327**: 3.3V compatible
- **Thermal Printer**: 3.3V or 5V TTL (check printer datasheet)

### Ground Connections

All components must share a common ground:
- ESP32 GND
- Printer GND
- Display GND
- Microphone GND
- Power supply GND

### ESD Protection

- Button should have 100nF capacitor to ground (optional, for noise immunity)
- Consider TVS diodes on UART lines if printer cable is long (>30cm)

## Spare Pins (Future Expansion)

Four spare GPIOs are reserved for future features:

- **GPIO 13**: Available for expansion
- **GPIO 14**: Available for expansion
- **GPIO 15**: Available for expansion (previously used for status LED, now repurposed)
- **GPIO 16**: Available for expansion

Potential uses:
- Wi-Fi provisioning button (long-press reset)
- External storage (SD card SPI)
- Additional status indicators (RGB LED ring, buzzer, etc.)
- Secondary UART for debugging
- NFC reader for user identification

## GPIO Restrictions (ESP32-S3)

### Strapping Pins

Be aware of these ESP32-S3 strapping pins (not used in our design):
- GPIO 0: Boot mode selection
- GPIO 3: JTAG enable
- GPIO 45: VDD_SPI voltage
- GPIO 46: ROM message printing

### USB Pins (If Using Native USB)

- GPIO 19: USB D- (not used in our design)
- GPIO 20: USB D+ (not used in our design)

### Flash Pins (Reserved)

The following pins are used for internal flash and should not be used:
- GPIO 26-32: SPI flash (QSPI mode)

## Schematic Symbol

```
ESP32-S3 (Top View)
┌─────────────────────┐
│                     │
│  GPIO 4  ────────── │ Mic BCLK
│  GPIO 5  ────────── │ Mic WS
│  GPIO 6  ────────── │ Mic DATA
│                     │
│  GPIO 12 ────────── │ Button (pull-up)
│  GPIO 13 ────────── │ Spare
│  GPIO 14 ────────── │ Spare
│  GPIO 15 ────────── │ Spare
│  GPIO 16 ────────── │ Spare
│  GPIO 17 ────────── │ Printer TX
│  GPIO 18 ────────── │ Printer RX
│                     │
│  GPIO 41 ────────── │ OLED SDA
│  GPIO 42 ────────── │ OLED SCL
│                     │
│  3V3     ────────── │ Power
│  GND     ────────── │ Ground
│                     │
└─────────────────────┘
```

## References

- [ESP32-S3 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf)
- [INMP441 Datasheet](https://www.invensense.com/wp-content/uploads/2015/02/INMP441.pdf)
- [SSD1327 Datasheet](https://www.waveshare.com/w/upload/c/c2/SSD1327.pdf)
- [ESC/POS Command Reference](https://reference.epson-biz.com/modules/ref_escpos/)

## Revision History

| Date       | Version | Changes                          |
|------------|---------|----------------------------------|
| 2026-02-05 | 1.1     | Removed LED (GPIO 15), plan for OLED display |
| 2026-02-05 | 1.0     | Initial pinout definition        |

---

**Last Updated**: February 5, 2026  
**Target Hardware**: ESP32-S3 (any variant with sufficient GPIO)
