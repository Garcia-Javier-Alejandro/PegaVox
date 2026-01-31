# PegaVox Tech Stack & Bill of Materials (BOM)

## Tech Stack

### Backend
- **Language:** Python 3.10+
- **Framework:** FastAPI (async, type-safe, easy docs)
- **Speech-to-Text:** OpenAI Whisper API (cloud, high accuracy, Spanish supported)
  - *Alternatives:* Local Whisper (whisper.cpp), Google Speech-to-Text, Azure Speech, Vosk
- **Image Generation:** OpenAI DALL·E API (cloud, reliable, creative)
  - *Alternatives:* Stable Diffusion (local/cloud), Replicate.com
- **Image Processing:** Pillow (Python, for resizing, dithering, 1-bit conversion)
  - *Alternatives:* OpenCV, Wand
- **Audio Recording (PC prototyping):** sounddevice or pyaudio (Python)
- **HTTP Client:** requests (Python)
- **Data Models:** Pydantic (for FastAPI schemas)

### Device (ESP32-S3)
- **Firmware:** ESP-IDF (C/C++)
- **Audio Input:** I2S interface for MEMS microphone (INMP441)
- **Networking:** Wi-Fi (HTTP client for backend API)
- **Display/UI:** SSD1327 OLED (SPI/I2C)
- **Button Input:** GPIO (arcade push button)
- **Printer Interface:** Direct hardware control (reverse-engineered, not vendor app)

### General Principles
- All AI/ML runs on backend (ESP32-S3 streams audio, receives raster data)
- Spanish language is fully supported by chosen APIs
- Cost discipline: only essential components, no vendor lock-in

---

## Bill of Materials (BOM)

| Category          | Item                                                                  |  Unit Cost | Notes                                                     | Link                                                                                                                                                                                                                                                                                                                                                         |
| ----------------- | --------------------------------------------------------------------- | ---------: | --------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| Compute           | ESP32-S3 Super Mini Dev Board (Wi-Fi + BT, MicroPython)               | 14,919 ARS | Chosen dev board                                          | [https://www.mercadolibre.com.ar/esp32-s3-super-mini-wifi-bt-placa-desarrollo-iot-micropython/up/MLAU3187034930](https://www.mercadolibre.com.ar/esp32-s3-super-mini-wifi-bt-placa-desarrollo-iot-micropython/up/MLAU3187034930)                                                                                                                             |
| Display           | OLED 1.5″ 128×128 (square)                                            | 29,469 ARS | Square UI, SSD1327                                        | [https://www.mercadolibre.com.ar/pantalla-dislay-oled-waveshare-15p-128x128-pixels-spi-i2c/up/MLAU240676387](https://www.mercadolibre.com.ar/pantalla-dislay-oled-waveshare-15p-128x128-pixels-spi-i2c/up/MLAU240676387)                                                                                                                                     |
| Audio Input       | INMP441 I2S MEMS Microphone (unit cost from 3-pack)                   | 30,166 ARS | 3-pack price ÷3                                           | [https://www.mercadolibre.com.ar/modulo-microfono-omnidireccional-inmp441-aitrip-3pcs-i2s/up/MLAU3323138834?matt_tool=38087446&pdp_filters=item_id%3AMLA2198781454&from=gshop](https://www.mercadolibre.com.ar/modulo-microfono-omnidireccional-inmp441-aitrip-3pcs-i2s/up/MLAU3323138834?matt_tool=38087446&pdp_filters=item_id%3AMLA2198781454&from=gshop) |
| UI Controls       | Arcade Push Button 30 mm (pink)                                       |  2,378 ARS | Momentary                                                 | [https://candy-ho.com/producto/boton-pulsador-arcade-30mm-push-rosa/](https://candy-ho.com/producto/boton-pulsador-arcade-30mm-push-rosa/)                                                                                                                                                                                                                   |
| Media             | Thermal adhesive roll 57 mm × 45 m (transparent)                      |  6,990 ARS | Sticker media                                             | [https://www.mercadolibre.com.ar/rollo-papel-termico-autoadhesivo-transparente-57mm-x-45m-ibi-craft/p/MLA41912380](https://www.mercadolibre.com.ar/rollo-papel-termico-autoadhesivo-transparente-57mm-x-45m-ibi-craft/p/MLA41912380)                                                                                                                         |
| Thermal Mgmt      | Aluminum thermal adhesive heatsink 40×30×5 mm                         |  1,899 ARS | Optional                                                  | [https://www.mercadolibre.com.ar/disipador-aluminio-adhesivo-termico-40x30x5-raspberry-hobb/up/MLAU224732151](https://www.mercadolibre.com.ar/disipador-aluminio-adhesivo-termico-40x30x5-raspberry-hobb/up/MLAU224732151)                                                                                                                                   |
| Printing          | Cat thermal printer (reuse electronics/controller; **no iPrint app**) | 20,000 ARS | We’ll drive it directly (hardware interface), not via app | [https://es.aliexpress.com/item/1005001666191411.html?gatewayAdapt=glo2esp](https://es.aliexpress.com/item/1005001666191411.html?gatewayAdapt=glo2esp)                                                                                                                                                                                                       |
| Power – Regulator | Buck DC-DC LM2596 (3 A)                                               |  4,269 ARS | 5V → 3.3V rail                                            | [https://listado.mercadolibre.com.ar/lm2596-step-down-3a](https://listado.mercadolibre.com.ar/lm2596-step-down-3a)                                                                                                                                                                                                                                           |
| Structure         | Perfboard FR4 double-sided 5×7 cm                                     |  5,890 ARS | 2 × 2,945 ARS                                             | [https://www.mercadolibre.com.ar/placa-experimental-pcb-perforada-5x7-doble-fr4-proto-ptec/up/MLAU352722226](https://www.mercadolibre.com.ar/placa-experimental-pcb-perforada-5x7-doble-fr4-proto-ptec/up/MLAU352722226)                                                                                                                                     |
| Power – Supply    | Switching PSU 5V 3A USB (FullEnergy)                                  |  9,600 ARS | Chosen supply                                             | [https://www.mercadolibre.com.ar/fuente-switching-5volt-3amp-salida-usb-fullenergy-de-pared/up/MLAU173327222](https://www.mercadolibre.com.ar/fuente-switching-5volt-3amp-salida-usb-fullenergy-de-pared/up/MLAU173327222)                                                                                                                                   |
| Misc              | Connectors, headers, wiring, consumables                              |  3,000 ARS | Fixed                                                     | N/A                                                                                                                                                                                                                                                                                                                                                          |

---

*All prices are approximate and for reference only. See links for current pricing and availability.*
