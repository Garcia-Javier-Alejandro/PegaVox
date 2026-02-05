# PegaVox 🎨🔊

**PegaVox** is a voice-driven sticker printer for children. Kids press a button, speak a prompt in natural language, and the device generates an AI-powered pixel-art sticker and prints it on a thermal printer.

## Features

- **🎤 Voice Input**: Capture natural language prompts via microphone
- **🧠 Speech-to-Text**: Transcribe Spanish language audio with OpenAI Whisper API
- **🎨 AI Image Generation**: Generate pixel-art style images using OpenAI DALL·E
- **🔒 Content Moderation**: Built-in safety gates using OpenAI Moderation API
- **🖨️ Thermal Printer Integration**: Print stickers on 58mm thermal printers
- **🖼️ Image Processing**: Automatic dithering, resizing, and 1-bit conversion for thermal output
- **📋 ESC/POS Compatible**: Direct thermal printer command generation
- **🌐 Prompt Engineering**: Optimized prompts for consistent, child-safe pixel-art output

## MVP Status: ✅ Functional

The core MVP pipeline is **fully functional** and tested:

```
Record Audio → Transcribe (Spanish) → Moderation Gate → 
Build Prompt → Generate Image → Post-Process → Print
```

**Test it:**
```bash
python scripts/pipeline.py --seconds 5 --out-prefix test --debug
```

This end-to-end script demonstrates all core features:
- Records audio from your microphone
- Transcribes it to text
- Validates content safety
- Generates an AI image
- Converts to 1-bit dithered format
- Outputs thermal printer-ready files

## Architecture

### Backend (Python)
- **pipeline.py**: End-to-end script combining all steps
- **Dependencies**: OpenAI API, Pillow, NumPy, sounddevice
- **Output Formats**:
  - `.final.png` – Final 1-bit dithered image
  - `.final.bitmap.bin` – Raw packed 1-bit bitmap (MSB-first)
  - `.final.escpos.bin` – ESC/POS GS v 0 thermal printer command stream
  - `.output.wav`, `.transcription.txt`, `.prompt.txt`, `.generated.png` (debug only)

### Device (ESP32-S3)
- **Firmware Status**: Phase 2 test (button → print via UART) ✅
- Captures audio via I2S MEMS microphone (planned)
- Sends to backend over HTTP (planned)
- Receives printer-ready raster data (planned)
- Streams to thermal printer hardware via UART (basic test working)

### Hardware (BOM)
- ESP32-S3 microcontroller
- INMP441 I2S MEMS microphone
- SSD1327 OLED 128×128 display
- 30mm arcade push button
- 58mm thermal printer (Catprinter or similar)
- Buck regulator (5V → 3.3V)
- Thermal adhesive sticker roll 57×45mm

*See [docs/tech-stack-and-bom.md](docs/tech-stack-and-bom.md) for full details and pricing.*

## Getting Started (PC Demo)

### Prerequisites
```bash
pip install -U openai pillow sounddevice numpy python-dotenv
```

### Environment
```bash
export OPENAI_API_KEY="sk-..."
```

### Run the Pipeline
```bash
python scripts/pipeline.py --seconds 5 --out-prefix my_test --debug
```

**Arguments:**
- `--seconds`: Recording duration (1–30, default 5)
- `--out-prefix`: Output filename prefix (default "run")
- `--printer-width`: Target width in pixels (default 384)
- `--pixelate-width`: Downscale for pixel emphasis (default 96, use 0 to disable)
- `--debug`: Save intermediate files (wav, transcription, prompt, generated PNG)

**Output** appears in `scripts/output/`:
```
20260202_110105_test.final.png
20260202_110105_test.final.bitmap.bin
20260202_110105_test.final.escpos.bin
```

Keeps only the 3 most recent runs.

## TODO: Path to Production Device

### Phase 0: Current Status

Currently runs locally in a Python dev environment to generate images from a spoken prompt. Audio is captured from the PC microphone, and there is no printer integration yet.

### Phase 1: Printer Bring-Up & Print Contract (Highest Risk)

**Goal:** Prove that the device can reliably print backend-generated images.

- [ ] **Confirm Printer Variant**
  - Cashino printer with TTL UART interface (not USB / RS232)
  - Confirm baud rate, voltage levels, status lines (paper out / busy)

- [ ] **Printer Power & Electrical Validation**
  - Dedicated power supply sized for peak current
  - Common ground between printer and ESP32-S3
  - Brownout testing during repeated prints

- [ ] **ESC/POS Raster Contract (Device ↔ Backend)**
  - Fixed width: 384 px (58 mm paper)
  - Height: variable
  - Format: ESC/POS GS v 0 raster command
  - 1-bit packed rows, MSB-first, black=1
  - Backend produces printer-ready bytes; device only streams

- [ ] **Printer Stress Testing**
  - Print known test patterns (checkerboard, solid fills, thin lines)
  - 100+ consecutive prints without corruption or resets

### Phase 2: Core Embedded Firmware (ESP32-S3)

**Goal:** Stable networking + printing foundation.

**Status:** ✅ Initial test firmware complete (Button → Print "Hello world")

- [x] **UART Printer Driver** (Basic)
  - ESC/POS command generation
  - Text printing with line feeds
  - Button-triggered print test
  - TODO: Buffered writes with pacing, status reads, abort logic

- [ ] **Wi-Fi & HTTPS Client**
  - Robust reconnect logic
  - Timeouts and retries with exponential backoff
  - Non-blocking operation during print/UI updates

- [ ] **Device Identity & Authentication**
  - Unique device_id per unit
  - Secure device token stored in NVS
  - Backend authorization for all API calls
  - No OpenAI keys on the device

- [ ] **OTA Firmware Updates**
  - Signed firmware images
  - Dual-partition with rollback
  - Safe-mode boot on repeated failure

**Current Test:** See [device/firmware/README.md](device/firmware/README.md) for build instructions.

### Phase 3: Wi-Fi Provisioning & First-Boot UX

**Goal:** Allow non-technical users to connect the device to Wi-Fi.

- [ ] **BLE-Based Wi-Fi Provisioning**
  - ESP32 advertises in provisioning mode on first boot
  - Secure BLE channel for SSID/password transfer
  - Store credentials securely in NVS

- [ ] **OLED-Guided Setup Flow**
  - Display clear step-by-step instructions
  - Show device name / pairing instructions
  - Feedback for success / failure

- [ ] **Provisioning Controls**
  - Long-press button to reset Wi-Fi credentials
  - Timeout → return to provisioning mode if connection fails

### Phase 4: Audio Capture & User Interaction

**Goal:** Predictable, child-friendly interaction.

- [ ] **I2S Audio Capture**
  - INMP441 microphone
  - Fixed recording window (e.g. 7 s)
  - On-device silence trimming / VAD

- [ ] **UI State Machine**
  - States: IDLE → LISTENING → UPLOADING → PROCESSING → PRINTING → DONE
  - Clear error states and automatic recovery
  - Cancel operation via long button press

- [ ] **OLED Display**
  - I2C SSD1327 driver
  - Simple, readable status messages
  - No blocking calls in UI loop

### Phase 5: Backend Integration & Hardening

**Goal:** Predictable cost, safety, and latency.

- [ ] **API Contract Enforcement**
  - Idempotent audio uploads
  - Job polling with TTL and expiry
  - Explicit error codes for device UI

- [ ] **Safety & Moderation**
  - Always-on local heuristic moderation
  - Optional API moderation via compile-time constant
  - Image endpoint moderation enabled

- [ ] **Cost Guardrails**
  - Per-device rate limits
  - Daily / monthly spend caps
  - Automatic shutdown on anomaly

- [ ] **Observability**
  - Latency breakdown (ASR / image / raster)
  - Error tracking and print success rate

### Phase 6: Mechanical Integration & Electrical Robustness

**Goal:** Turn electronics into a reliable product.

- [ ] **Mechanical Integration**
  - Mount ESP32-S3, mic, button, OLED
  - Paper access and serviceability
  - Thermal isolation from print head

- [ ] **EMI / Noise Mitigation**
  - Separate printer and MCU power rails
  - Filtering to protect microphone input
  - Grounding strategy validation

### Phase 7: Validation & Pre-Production

**Goal:** Ensure reliability before wider deployment.

- [ ] **End-to-End Soak Testing**
  - Hundreds of print cycles
  - Wi-Fi drop / reconnect scenarios
  - Power interruption during print

- [ ] **Child-Safety Review**
  - Prompt edge cases
  - Image tone and consistency
  - UX clarity on errors

- [ ] **Golden Test Vectors**
  - Known audio → known raster hash
  - Regression tests for backend updates

## Project Structure

```
PegaVox/
├── backend/
│   ├── app/               (FastAPI service – TODO)
│   └── tests/
├── device/
│   ├── firmware/          (ESP-IDF / ESP32-S3 code – ✅ Phase 2 test)
│   │   ├── src/
│   │   │   └── main.cpp            (App entry point: button → print flow)
│   │   ├── include/
│   │   │   ├── ThermalPrinter.hpp  (ESC/POS printer driver interface)
│   │   │   └── Button.hpp          (Debounced button handler interface)
│   │   ├── lib/
│   │   │   ├── ThermalPrinter/
│   │   │   │   ├── ThermalPrinter.cpp  (Printer driver implementation)
│   │   │   │   └── library.json        (Library metadata)
│   │   │   └── Button/
│   │   │       ├── Button.cpp          (Button handler implementation)
│   │   │       └── library.json
│   │   ├── platformio.ini          (PlatformIO project config)
│   │   ├── README.md               (Firmware build and test instructions)
│   │   ├── .gitignore              (Ignore build artifacts)
│   │   └── secrets_example.hpp     (Template for Wi-Fi credentials)
│   └── hardware/          (Schematics, PCB – TODO)
├── scripts/
│   ├── pipeline.py        (PC end-to-end demo – ✅ working)
│   └── output/            (Generated files)
├── docs/
│   ├── mvp-checklist.md
│   ├── backend-device-api-contract.md
│   ├── tech-stack-and-bom.md
│   └── pinout.md          (ESP32-S3 pin mapping – ✅)
└── images/                (Reference images, diagrams)
```

## API Contract (Backend ↔ Device)

See [docs/backend-device-api-contract.md](docs/backend-device-api-contract.md) for the full specification.

**Quick Reference:**

```
POST /api/v1/audio
Content-Type: audio/wav
Body: Raw WAV bytes (5–10 seconds)
Response: 202 Accepted + { "job_id": "..." }

GET /api/v1/job/{job_id}
Response: 
  { "status": "processing" }
  or { "status": "done", "raster_data": "<base64-bitmap>" }
  or { "status": "error", "message": "..." }
```

## Troubleshooting (Development)

### `IndentationError` in `pipeline.py`
Ensure all code is properly indented. The end-to-end blocks (pack bits, write ESC/POS, cleanup, print) must be inside the `main()` function.

### `OPENAI_API_KEY` not set
```bash
export OPENAI_API_KEY="sk-..."
python scripts/pipeline.py --seconds 5 --out-prefix test
```

### Audio not recording
Check that your default microphone is selected and permissions are granted.

## License

[Specify your license, e.g., MIT, GPL-3.0, etc.]

## Contributing

Contributions welcome! Focus areas:
- Embedded firmware (ESP-IDF)
- Thermal printer reverse-engineering
- Backend FastAPI service
- UI/UX polish

---

**Made with ❤️ for creative kids everywhere.**
