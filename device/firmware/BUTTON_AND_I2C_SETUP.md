# Button and I2C Setup Guide

## Part A: Button Configuration (GPIO 12, Active-Low)

### Hardware Explanation

**Wiring:**
```
GPIO12 ──── [Button] ──── GND
```

No external resistor needed—ESP32-S3 internal pull-up will hold GPIO12 HIGH when the button is open.

**Logic Levels:**
- **Idle (button open):** GPIO12 reads HIGH (pulled to 3.3V internally)
- **Pressed (button closed):** GPIO12 reads LOW (connected to GND)

### Firmware Configuration (ESP-IDF)

```cpp
#include "driver/gpio.h"

// Button configuration
#define BUTTON_PIN          GPIO_NUM_12
#define BUTTON_DEBOUNCE_MS  50

void button_init(void)
{
    gpio_config_t config = {
        .pin_bit_mask = (1ULL << BUTTON_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,    // Enable internal pull-up
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE,      // Trigger on falling edge (press)
    };
    
    ESP_ERROR_CHECK(gpio_config(&config));
}

// Simple debouncing loop
bool button_is_pressed(void)
{
    static TickType_t last_press_time = 0;
    TickType_t now = xTaskGetTickCount();
    
    // Check if button is LOW (pressed)
    if (gpio_get_level(BUTTON_PIN) == 0) {
        // Debounce: only register if enough time has passed
        if ((now - last_press_time) > pdMS_TO_TICKS(BUTTON_DEBOUNCE_MS)) {
            last_press_time = now;
            return true;
        }
    }
    return false;
}
```

### Firmware Configuration (Arduino)

```cpp
#define BUTTON_PIN 12
#define BUTTON_DEBOUNCE_MS 50

void setup()
{
    // Arduino: INPUT_PULLUP enables internal pull-up
    pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void loop()
{
    static unsigned long last_press_time = 0;
    unsigned long now = millis();
    
    // Button is pressed when pin reads LOW
    if (digitalRead(BUTTON_PIN) == LOW) {
        if ((now - last_press_time) > BUTTON_DEBOUNCE_MS) {
            Serial.println("Button pressed!");
            last_press_time = now;
            // Handle button press here
        }
    }
}
```

### Important: Check for GPIO Strapping Pin Conflicts

⚠️ **GPIO 12 is NOT a strapping pin**, so it's safe from boot-mode conflicts.

However, **if you observe boot issues while holding the button**, consider these alternatives:
- **GPIO 13**: Safe, unused spare pin
- **GPIO 14**: Safe, unused spare pin
- **GPIO 15**: Safe, unused spare pin (freed from LED)
- **GPIO 16**: Safe, unused spare pin

---

## Part B: I2C Configuration (GPIO 41 = SDA, GPIO 42 = SCL)

### Hardware Explanation

**I2C is an open-drain bus.**

```
         3.3V
          │
         Rpu (pull-up resistor)
          │
GPIO41 ───┴──── [SDA line] ──── (OLED and other devices)
GPIO42 ───┴──── [SCL line] ──── (OLED and other devices)
```

**Why pull-ups are required:**
- I2C devices (like OLED) have open-drain outputs (can only pull LOW, not HIGH)
- Without pull-ups, the bus cannot return to HIGH state
- **Result: Bus is stuck LOW, no communication possible**

### Finding Existing Pull-Ups on OLED Breakout

**Most SSD1327 breakouts ALREADY INCLUDE 4.7kΩ pull-ups on SDA and SCL.**

**To verify visually:**
1. Examine the PCB for resistors near the I2C connector
2. Look for component markings: `"472"` or `"4.7"` = 4.7kΩ
3. Check the breakout schematic (often printed on the PCB)

**To verify with a multimeter:**
1. Power off the device
2. Set multimeter to resistance (Ω)
3. Measure SDA resistance to 3.3V: should be ~4.7kΩ (open-circuit to 10kΩ)
4. Measure SCL resistance to 3.3V: should be ~4.7kΩ

**Result:**
- If resistance reads 4.7kΩ → pull-ups present ✅ (no external resistors needed)
- If resistance reads >20kΩ or open circuit → no pull-ups ❌ (add 4.7kΩ externally)

### Adding External Pull-Ups (if needed)

If your OLED breakout **lacks pull-ups**, add them:

```
3.3V ──── [4.7kΩ] ──── GPIO41 (SDA)
3.3V ──── [4.7kΩ] ──── GPIO42 (SCL)
```

**Soldering tips:**
- Use 1/4W or 1/8W resistors
- Solder between 3.3V pad and SDA/SCL pads on breakout
- Keep leads short (< 2 inches)
- Use 100nF decoupling capacitors near OLED power if noise is observed

### Firmware Configuration (ESP-IDF)

```cpp
#include "driver/i2c.h"

#define I2C_MASTER_SCL_IO   GPIO_NUM_42
#define I2C_MASTER_SDA_IO   GPIO_NUM_41
#define I2C_MASTER_NUM      I2C_NUM_0      // Use I2C0
#define I2C_MASTER_FREQ_HZ  400000         // 400 kHz (Fast Mode)
#define I2C_MASTER_TX_BUF_DISABLE 0
#define I2C_MASTER_RX_BUF_DISABLE 0

void i2c_init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,  // Optional: use if no external pull-ups
        .scl_pullup_en = GPIO_PULLUP_ENABLE,  // Optional: use if no external pull-ups
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
        .clk_flags = 0,
    };
    
    ESP_ERROR_CHECK(i2c_param_config(I2C_MASTER_NUM, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_MASTER_NUM, 
                                       conf.mode, 
                                       I2C_MASTER_RX_BUF_DISABLE,
                                       I2C_MASTER_TX_BUF_DISABLE, 
                                       0));
    
    ESP_LOGI(TAG, "I2C initialized on GPIO %d (SDA) and GPIO %d (SCL)",
             I2C_MASTER_SDA_IO, I2C_MASTER_SCL_IO);
}

// Simple I2C bus scan (verify OLED detection)
void i2c_scan(void)
{
    esp_err_t ret;
    uint8_t address;
    printf("I2C Scanner:\n");
    printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\r\n");
    
    for (int i = 0; i < 128; i += 16) {
        printf("%02x: ", i);
        for (int j = 0; j < 16; j++) {
            address = i + j;
            i2c_cmd_handle_t cmd = i2c_cmd_link_create();
            i2c_master_start(cmd);
            i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, true);
            i2c_master_stop(cmd);
            ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 50 / portTICK_PERIOD_MS);
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
```

### Firmware Configuration (Arduino)

```cpp
#include <Wire.h>

#define I2C_SDA_PIN 41
#define I2C_SCL_PIN 42

void setup()
{
    Serial.begin(115200);
    
    // Initialize I2C with custom pins
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, 400000);  // 400 kHz
    
    Serial.println("I2C initialized");
    i2c_scan();
}

// Simple I2C scanner
void i2c_scan()
{
    Serial.println("\nI2C Scanner:");
    int count = 0;
    
    for (byte i = 8; i < 120; i++) {
        Wire.beginTransmission(i);
        if (Wire.endTransmission() == 0) {
            Serial.print("Found device at 0x");
            Serial.println(i, HEX);
            count++;
        }
    }
    
    Serial.print("Total devices found: ");
    Serial.println(count);
}
```

### Reliability Tips

1. **Keep wires short** (< 4 inches / 10 cm)
   - I2C is susceptible to capacitive coupling at high clock rates
   - Longer wires = more EMI, potential signal integrity issues

2. **Always pull to 3.3V, never 5V**
   - GPIO41/42 are 3.3V logic
   - Pulling to 5V risks damaging the ESP32-S3

3. **Avoid bus lockups**
   - If OLED becomes unresponsive, implement a bus reset:

```cpp
void i2c_bus_reset(void)
{
    // Clock SCL 9 times while monitoring SDA
    // This helps devices release stuck states
    gpio_config_t scl_config = {
        .pin_bit_mask = (1ULL << I2C_MASTER_SCL_IO),
        .mode = GPIO_MODE_OUTPUT_OD,  // Open-drain
        .pull_up_en = GPIO_PULLUP_DISABLE,
    };
    gpio_config(&scl_config);
    
    for (int i = 0; i < 9; i++) {
        gpio_set_level(I2C_MASTER_SCL_IO, 0);
        vTaskDelay(pdMS_TO_TICKS(5));
        gpio_set_level(I2C_MASTER_SCL_IO, 1);
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    
    // Reinitialize I2C driver
    i2c_driver_delete(I2C_MASTER_NUM);
    i2c_init();
}
```

4. **Add decoupling capacitors** (optional but recommended)
   - 100nF ceramic cap from OLED VCC → GND (near the OLED module)
   - Reduces noise and glitches

---

## Validation Checklist

- [ ] **Button Wiring**
  - [ ] GPIO 12 connected to one side of pushbutton
  - [ ] Other side of pushbutton connected to GND
  - [ ] No external resistor needed
  - [ ] Test with multimeter: button reads ~0Ω when pressed, >10kΩ when open

- [ ] **OLED Power**
  - [ ] OLED VCC connected to 3.3V
  - [ ] OLED GND connected to ESP32 GND (common ground!)
  - [ ] 100nF decoupling cap near OLED (optional but recommended)

- [ ] **I2C Wiring**
  - [ ] GPIO 41 (SDA) connected to OLED SDA
  - [ ] GPIO 42 (SCL) connected to OLED SCL
  - [ ] **Pull-ups verified** (check for 4.7kΩ to 3.3V, or add if missing)
  - [ ] Wires < 4 inches long
  - [ ] No 5V applied to I2C lines

- [ ] **Firmware Initialization**
  - [ ] `button_init()` called in `app_main()`
  - [ ] `i2c_init()` called in `app_main()`
  - [ ] Serial monitor shows "I2C initialized" message
  - [ ] `i2c_scan()` detects OLED at 0x3C

- [ ] **Functional Tests**
  - [ ] Press button → serial log prints "Button pressed"
  - [ ] Vary debounce time → no spurious button presses
  - [ ] I2C scan finds OLED at 0x3C ✅
  - [ ] OLED display initializes without errors

---

## References

- [ESP32-S3 I2C Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/peripherals/i2c.html)
- [SSD1327 I2C Protocol](https://www.waveshare.com/datasheet/LCD_zh/SSD1327.pdf)
- [I2C Pull-Up Theory](https://www.nxp.com/docs/en/user-guide/UM10204.pdf)
- [ESP32-S3 GPIO Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/peripherals/gpio.html)

---

**Last Updated:** February 5, 2026
