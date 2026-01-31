# PegaVox Backend–Device API Contract (Draft)

This document defines the minimal API contract between the ESP32-S3 thin client and the backend server for the PegaVox prototype. The goal is to support voice-driven sticker printing with a low-cost thermal printer, minimizing device complexity and handling all AI/image processing on the backend.

---

## System Overview
- **ESP32-S3**: Captures audio, sends to backend, receives printer-ready raster data, streams to printer.
- **Backend**: Handles speech-to-text, prompt generation, image creation, dithering/rasterization, and returns printer-ready data.

---

## API Contract (v0.1 – Draft)

### 1. Audio Upload
- **Endpoint:** `POST /api/v1/audio`
- **Request:**
  - Content-Type: `audio/wav` (or `audio/flac`)
  - Body: Raw audio data (single utterance, e.g., 2–10 seconds)
- **Response:**
  - `202 Accepted` + JSON: `{ "job_id": "string" }`

### 2. Job Status & Result
- **Endpoint:** `GET /api/v1/job/{job_id}`
- **Response:**
  - If processing: `{ "status": "processing" }`
  - If done: `{ "status": "done", "raster_data": "<base64-encoded-binary>" }`
  - If error: `{ "status": "error", "message": "string" }`

### 3. Raster Data Format
- **Type:** 1-bit-per-pixel, left-to-right, top-to-bottom, width and height fixed per printer model (e.g., 384 px wide)
- **Encoding:** Binary, base64-encoded in JSON
- **No printer protocol framing**: Device is responsible for low-level timing and signaling, backend only provides raw bitmap.

---

## Design Principles
- **Minimal device logic:** ESP32-S3 only streams audio and prints raster data.
- **No vendor lock-in:** No use of proprietary apps or cloud.
- **Exploratory printer integration:** Backend does not assume ESC/POS or other protocols; device firmware adapts as needed.
- **Cost discipline:** No unnecessary features or hardware.

---

## Open Questions
- How to handle printer width/height negotiation?
- Should backend support multiple output formats (e.g., PNG for debugging)?
- How to handle streaming/large jobs?

---

## Next Steps
- Prototype backend endpoints with mocked data.
- Develop minimal ESP32-S3 client for audio upload and raster download.
- Begin reverse-engineering printer interface and document findings.

---

*This contract is a living document and will evolve as integration progresses.*
