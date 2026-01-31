# PegaVox MVP Milestone Checklist

This checklist defines the practical, incremental steps to build a minimal working prototype of PegaVox: a voice-driven sticker printer. The goal is to achieve an end-to-end loop: voice input → transcription → prompt → image → output (file/display).

---

## Milestone 1: Record Audio from Microphone
- **Goal:** Capture a short audio clip from the PC microphone and save it as a WAV file.
- **Acceptance Criteria:**
  - User can run a script to record audio and save it (e.g., `output.wav`).
  - Audio file plays back clearly.
- **Test Command:**
  - `python record_audio.py`

---

## Milestone 2: Transcribe Audio to Text (Speech-to-Text)
- **Goal:** Convert the recorded audio file to text using OpenAI's Whisper API.
- **Acceptance Criteria:**
  - Script takes an audio file and prints the transcribed text.
  - Handles short, clear speech reliably.
- **Test Command:**
  - `python transcribe_audio.py output.wav`

---

## Milestone 3: Generate Image from Text Prompt
- **Goal:** Use the transcribed text as a prompt to generate a small (≈384×384 px), black-and-white, pixel-art-style image using OpenAI's image API.
- **Acceptance Criteria:**
  - Script sends a text prompt and receives an image file (e.g., `output.png`).
  - Image is 384×384 px, black-and-white, and visually matches the prompt.
- **Test Command:**
  - `python generate_image.py "A cat in pixel art"`

---

## Milestone 4: Display or Save the Generated Image
- **Goal:** Show the generated image on screen or save it to disk for review.
- **Acceptance Criteria:**
  - Script displays the image in a window or saves it to a file.
  - User can visually confirm the result.
- **Test Command:**
  - `python display_image.py output.png`

---

## Milestone 5: End-to-End Demo Script
- **Goal:** Integrate all steps into a single script: record audio → transcribe → generate image → display/save.
- **Acceptance Criteria:**
  - User runs one command and receives an image based on their spoken description.
  - Handles basic errors (e.g., no speech detected).
- **Test Command:**
  - `python pegavox_demo.py`

---

## (Optional) Milestone 6: Prepare Image for Thermal Printer
- **Goal:** Convert the generated image to a 1-bit (black-and-white) bitmap suitable for thermal printing.
- **Acceptance Criteria:**
  - Script takes an image and outputs a 1-bit bitmap file.
  - Bitmap visually matches the original image.
- **Test Command:**
  - `python convert_to_bitmap.py output.png`

---

## Notes
- Each milestone should be tested independently before integration.
- Use simple, well-documented scripts for each step.
- Focus on minimal, working solutions before adding features or optimizations.
