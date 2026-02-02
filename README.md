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
- Minimal thin client (firmware under development)
- Captures audio via I2S MEMS microphone
- Sends to backend over HTTP
- Receives printer-ready raster data
- Streams to thermal printer hardware

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

### Phase 1: Embedded Software (ESP32-S3 Firmware)

- [ ] **I2S Audio Capture**
  - Initialize I2S peripheral for INMP441 microphone
  - Implement circular DMA buffer for audio streaming
  - Provide blocking/non-blocking recording APIs

- [ ] **Wi-Fi & HTTP Client**
  - Wi-Fi provisioning and connection management
  - HTTPS client for backend API communication
  - Robust retry logic and timeouts

- [ ] **Backend API Integration**
  - `POST /api/v1/audio` – Send 5–10 second audio clip
  - `GET /api/v1/job/{job_id}` – Poll for results (with exponential backoff)
  - Parse response and extract base64-encoded raster data

- [ ] **Button & UI States**
  - GPIO input for arcade button press
  - SSD1327 OLED display driver (SPI/I2C)
  - UI states: idle → listening → processing → printing → done

- [ ] **Memory & Performance**
  - Optimize audio buffering to fit in ESP32-S3 SRAM
  - Implement streaming audio upload (chunked HTTP POST)
  - Minimize latency from button press to "processing" indication

### Phase 2: Thermal Printer Hardware Interface

- [ ] **Reverse-Engineer Printer Interface**
  - Capture communication between printer controller and print head
  - Document pin layout and signaling protocol
  - Identify timing requirements and power draw

- [ ] **Direct Hardware Control (No Vendor App)**
  - Implement printer controller bypass (direct SPI/GPIO to print head)
  - or interface directly with existing printer electronics
  - Validate with test prints of known patterns

- [ ] **Thermal Print Driver**
  - Implement ESC/POS or raw thermal protocol in firmware
  - Handle media detection and auto-feed
  - Thermal element timing and power management

- [ ] **Mechanical Integration**
  - Mount ESP32-S3 + microphone + button on/near printer
  - Integrate OLED display into enclosure
  - Thermal dissipation for print head

### Phase 3: Device Assembly & Testing

- [ ] **Prototype Assembly**
  - Solder perfboard or custom PCB
  - Wire microphone (I2S), button (GPIO), display (SPI), printer (TBD)
  - Validate power delivery under peak load

- [ ] **Integration Testing**
  - End-to-end: button → record → transcribe → generate → print
  - Test with native Spanish speech patterns
  - Measure latency and user experience

- [ ] **Safety & Content Validation**
  - Verify moderation gate blocks inappropriate prompts
  - Stress test with adversarial inputs
  - Child-safety audit of generated images

- [ ] **User Experience Polish**
  - Button feedback (visual / haptic)
  - Status messages on OLED display
  - Error handling and user-friendly messages
  - Sticker output quality and consistency

### Phase 4: Backend Deployment

- [ ] **FastAPI Backend Service**
  - RESTful API for `/api/v1/audio` upload
  - Job queue and result caching
  - Configurable printer width and output format

- [ ] **Scaling & Monitoring**
  - Horizontal scaling for multiple devices
  - Logging and error tracking
  - Cost monitoring for OpenAI API usage

- [ ] **Documentation**
  - API contract finalized
  - Firmware integration guide
  - Deployment instructions

## Project Structure

```
PegaVox/
├── backend/
│   ├── app/               (FastAPI service – TODO)
│   └── tests/
├── device/
│   ├── firmware/          (ESP-IDF / ESP32-S3 code – TODO)
│   │   ├── secrets_example.h
│   │   └── secrets.h
│   └── hardware/          (Schematics, PCB – TODO)
├── scripts/
│   ├── pipeline.py        (PC end-to-end demo – ✅ working)
│   └── output/            (Generated files)
├── docs/
│   ├── mvp-checklist.md
│   ├── backend-device-api-contract.md
│   └── tech-stack-and-bom.md
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
