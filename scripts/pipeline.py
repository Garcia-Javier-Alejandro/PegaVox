# pipeline.py
#
# End-to-end: record audio -> transcribe (es) -> moderation gate -> prompt -> gpt-image-1 -> 58mm printer outputs
#
# Usage:
#   python pipeline.py --seconds 5 --out-prefix run1
#   python pipeline.py --seconds 5 --out-prefix run1 --debug
#
# Outputs (default):
#   run1.final.png          (384px wide, 1-bit dithered)
#   run1.final.bitmap.bin   (raw packed 1-bit rows, MSB first)
#   run1.final.escpos.bin   (ESC/POS GS v 0 raster command + data)
#
# Debug outputs (if --debug):
#   run1.output.wav
#   run1.transcription.txt
#   run1.prompt.txt
#   run1.generated.png      (original generated image from API)
#
# Requires:
#   pip install -U openai pillow sounddevice numpy
#
# Env:
#   OPENAI_API_KEY must be set

import argparse
import base64
import os

from dotenv import load_dotenv
load_dotenv()   # MUST happen before importing OpenAI

from openai import OpenAI

import sys
import time
import wave
from dataclasses import dataclass
from io import BytesIO
from typing import Optional, Tuple

import numpy as np
import sounddevice as sd
from PIL import Image





# ----------------------------
# Config / constants
# ----------------------------

SAMPLE_RATE = 16000
CHANNELS = 1
DTYPE = "int16"

TRANSCRIBE_MODEL = "whisper-1"
MODERATION_MODEL = "omni-moderation-latest"
IMAGE_MODEL = "gpt-image-1"

DEFAULT_PRINTER_WIDTH = 384  # common for 58mm ESC/POS printers


# ----------------------------
# Helpers
# ----------------------------

def eprint(*args, **kwargs):        # Same as print(), but sends output to stderr instead of stdout for logging/debug messages.
    print(*args, file=sys.stderr, **kwargs) 


def record_wav(seconds: float, wav_path: Optional[str] = None) -> bytes:
    """Record audio from default mic. Returns WAV bytes (RIFF). Optionally writes to wav_path."""
    if seconds <= 0:
        raise ValueError("seconds must be > 0")
    if seconds > 30:
        raise ValueError("Maximum recording duration is 30 seconds.")

    eprint(f"Recording {seconds:.1f}s @ {SAMPLE_RATE} Hz mono...")
    audio = sd.rec(int(seconds * SAMPLE_RATE), samplerate=SAMPLE_RATE, channels=CHANNELS, dtype=DTYPE)

    # simple countdown like your existing script
    for remaining in range(int(seconds), 0, -1):
        eprint(f"...{remaining} second(s) remaining...")
        time.sleep(1)

    sd.wait()
    audio = np.squeeze(audio)

    # Build WAV bytes in-memory
    bio = BytesIO()
    with wave.open(bio, "wb") as wf:
        wf.setnchannels(CHANNELS)
        wf.setsampwidth(2)  # int16
        wf.setframerate(SAMPLE_RATE)
        wf.writeframes(audio.tobytes())

    wav_bytes = bio.getvalue()

    if wav_path:
        with open(wav_path, "wb") as f:
            f.write(wav_bytes)

    return wav_bytes


def transcribe_es(client: OpenAI, wav_bytes: bytes) -> str:
    """Transcribe Spanish audio. Returns text."""
    # OpenAI SDK expects a file-like object with a name
    bio = BytesIO(wav_bytes)
    bio.name = "audio.wav"  # some clients rely on name for mime/type hints

    resp = client.audio.transcriptions.create(
        model=TRANSCRIBE_MODEL,
        file=bio,
        language="es",
    )
    # resp.text in newer SDKs
    text = getattr(resp, "text", None)
    if not text:
        raise RuntimeError("Transcription returned empty text.")
    return text.strip()


def is_flagged(client: OpenAI, text: str) -> bool:
    resp = client.moderations.create(
        model=MODERATION_MODEL,
        input=text
    )
    return bool(resp.results[0].flagged)


def build_pixel_art_prompt(subject_es: str) -> str:
    subject_es = " ".join(subject_es.strip().split())

    # Positive constraints first; minimal “no X” constraints only for common failure modes (text/realism).
    return f"""
Create a child-safe greyscale pixel art image.

STYLE (must follow):
- Pixel art with clearly visible square pixels (low-resolution look)
- 1-bit or 4-bit grayscale only (black/white or black/white + 2 greys)
- Cartoonish, not realistic
- Simple shapes, soft rounded edges
- Cheerful, harmless, friendly mood
- Sticker-like centered composition
- Plain light background

CONSTRAINTS:
- No text, letters, numbers, logos, watermarks
- No realism, no photographic details, no complex shading or gradients
- No violence, weapons, blood, fear, drugs, or adult themes

SUBJECT (Spanish transcription; interpret literally but keep it kid-safe):
{subject_es}
""".strip()


def generate_image_png_bytes(client: OpenAI, prompt: str, size: str = "1024x1024") -> bytes:
    """Generate image using gpt-image-1. Returns PNG bytes."""
    img = client.images.generate(
        model=IMAGE_MODEL,
        prompt=prompt,
        size=size,
        moderation="auto",
        output_format="png",
        quality="auto",
        n=1,
    )
    b64 = img.data[0].b64_json
    return base64.b64decode(b64)


def resize_to_width(img: Image.Image, width: int) -> Image.Image:
    """Resize keeping aspect ratio to target width."""
    w, h = img.size
    if w == width:
        return img
    new_h = max(1, int(round(h * (width / w))))
    return img.resize((width, new_h), Image.LANCZOS)


def pixelate(img: Image.Image, target_small_width: int) -> Image.Image:
    """
    Optional: enforce visible pixels by downscaling then upscaling with NEAREST.
    Useful if the model output is too smooth.
    """
    w, h = img.size
    if target_small_width <= 0 or target_small_width >= w:
        return img
    small_h = max(1, int(round(h * (target_small_width / w))))
    small = img.resize((target_small_width, small_h), Image.NEAREST)
    return small.resize((w, h), Image.NEAREST)


def to_1bit_dither(img: Image.Image) -> Image.Image:
    """
    Convert to 1-bit using Floyd–Steinberg dithering.
    """
    gray = img.convert("L")
    bw = gray.convert("1")  # default uses Floyd–Steinberg
    return bw


def pack_1bit_rows(img_1bit: Image.Image) -> Tuple[bytes, int, int, int]:
    """
    Pack 1-bit image into bytes, MSB first, row-major.
    Returns (data, width, height, bytes_per_row).
    """
    if img_1bit.mode != "1":
        raise ValueError("img_1bit must be mode '1'")

    w, h = img_1bit.size
    bytes_per_row = (w + 7) // 8

    # In mode '1', pixels are either 0 or 255. We treat "black" pixels as 1-bits for printing.
    # Pillow gives 0 for black, 255 for white when converted to L; in '1' it's still 0/255.
    pix = img_1bit.load()

    out = bytearray(bytes_per_row * h)
    for y in range(h):
        row_offset = y * bytes_per_row
        for x in range(w):
            byte_i = row_offset + (x // 8)
            bit = 7 - (x % 8)
            is_black = (pix[x, y] == 0)
            if is_black:
                out[byte_i] |= (1 << bit)
    return bytes(out), w, h, bytes_per_row


def escpos_gs_v_0(data: bytes, bytes_per_row: int, height: int) -> bytes:
    """
    ESC/POS raster bit image command (GS v 0).
    Format:
      GS v 0 m xL xH yL yH d1..dk
    Where x = bytes per row, y = height.
    m=0 normal.
    """
    xL = bytes_per_row & 0xFF
    xH = (bytes_per_row >> 8) & 0xFF
    yL = height & 0xFF
    yH = (height >> 8) & 0xFF
    header = bytes([0x1D, 0x76, 0x30, 0x00, xL, xH, yL, yH])
    return header + data


# ----------------------------
# Main
# ----------------------------

def main():
    parser = argparse.ArgumentParser(description="Audio->Transcribe(es)->Moderate->Prompt->gpt-image-1->Thermal outputs")
    parser.add_argument("--seconds", type=float, default=5.0, help="Recording duration in seconds (<=30)")
    parser.add_argument("--out-prefix", type=str, default="run", help="Prefix for output files")
    parser.add_argument("--printer-width", type=int, default=DEFAULT_PRINTER_WIDTH, help="Target printer width in pixels (commonly 384 for 58mm)")
    parser.add_argument("--pixelate-width", type=int, default=96, help="Downscale width before upscaling (NEAREST) to emphasize pixels; 0 disables")
    parser.add_argument("--debug", action="store_true", help="Write debug artifacts (wav, transcription, prompt, generated png)")

    args = parser.parse_args()

    if not os.getenv("OPENAI_API_KEY"):
        eprint("Error: Set OPENAI_API_KEY environment variable.")
        sys.exit(1)

    client = OpenAI()

    out_wav = f"{args.out_prefix}.output.wav"
    out_trans = f"{args.out_prefix}.transcription.txt"
    out_prompt = f"{args.out_prefix}.prompt.txt"
    out_generated = f"{args.out_prefix}.generated.png"

    out_final_png = f"{args.out_prefix}.final.png"
    out_bitmap = f"{args.out_prefix}.final.bitmap.bin"
    out_escpos = f"{args.out_prefix}.final.escpos.bin"

    # 1) record
    wav_bytes = record_wav(args.seconds, wav_path=out_wav if args.debug else None)

    # 2) transcribe (OpenAI call)
    eprint("Transcribing (es)...")
    transcription = transcribe_es(client, wav_bytes)
    eprint("Transcription:", transcription)

    if args.debug:
        with open(out_trans, "w", encoding="utf-8") as f:
            f.write(transcription)

    # 3) moderation gate (OpenAI call)
    eprint("Moderating transcription...")
    if is_flagged(client, transcription):
        print("flagged content, prompt cancelled")
        sys.exit(1)

    # 4) prompt (local)
    prompt = build_pixel_art_prompt(transcription)
    if args.debug:
        with open(out_prompt, "w", encoding="utf-8") as f:
            f.write(prompt)

    # 5) image gen (OpenAI call)
    eprint("Generating image (gpt-image-1)...")
    png_bytes = generate_image_png_bytes(client, prompt, size="1024x1024")
    if args.debug:
        with open(out_generated, "wb") as f:
            f.write(png_bytes)

    # 6) postprocess for printer
    img = Image.open(BytesIO(png_bytes)).convert("RGB")

    # Resize to printer width
    img = resize_to_width(img, args.printer_width)

    # Optional pixelation pass to force “visible pixels”
    if args.pixelate_width and args.pixelate_width > 0:
        img = pixelate(img, args.pixelate_width)

    # Convert to 1-bit with dithering
    bw = to_1bit_dither(img)

    # Save final PNG
    bw.save(out_final_png)

    # Pack bits + write raw bitmap
    bitmap, w, h, bpr = pack_1bit_rows(bw)
    with open(out_bitmap, "wb") as f:
        f.write(bitmap)

    # Write ESC/POS command stream
    escpos_bytes = escpos_gs_v_0(bitmap, bpr, h)
    with open(out_escpos, "wb") as f:
        f.write(escpos_bytes)

    print("OK")
    print(f"Saved: {out_final_png}")
    print(f"Saved: {out_bitmap} (raw packed 1-bit rows)")
    print(f"Saved: {out_escpos} (ESC/POS GS v 0 raster command)")

if __name__ == "__main__":
    main()
