"""
make_prompt.py

Generate a child-safe greyscale pixel-art prompt from an audio transcription
and generate the image via OpenAI's Images API (gpt-image-1).

Workflow:
1) Read transcription.txt
2) Optional local hard-fail check (free)
3) Moderation check (omni-moderation-latest). If flagged -> cancel.
4) Build prompt with strict style constraints
5) Generate image with gpt-image-1 with moderation="auto"

How to run (in /scripts; use only filenames, not paths):
    python make_prompt.py transcription.txt out.png [prompt.txt]

Requires:
    pip install -U openai
Environment:
    OPENAI_API_KEY must be set
"""

import base64
import re
import sys
import unicodedata
from typing import Optional

from openai import OpenAI


# ---- Config ----

IMAGE_MODEL = "gpt-image-1"
MODERATION_MODEL = "omni-moderation-latest"

# Keep this list SMALL. This is only a last-resort "obvious extreme" local block.
# Moderation is the primary gate and supports Spanish well.
HARD_FAIL_PHRASES = [
    # English
    "rape", "incest", "child porn", "child sexual", "bestiality", "necrophilia",
    # Spanish (common forms; normalization removes accents)
    "violacion", "incesto", "pornografia infantil", "abuso sexual infantil",
    "zoofilia", "necrofila",
]


# ---- Helpers ----

def normalize_text(text: str) -> str:
    """
    - Trims
    - Collapses whitespace
    - Normalizes unicode (so Spanish accents are handled consistently)
    """
    text = text.strip()
    text = re.sub(r"\s+", " ", text)
    return text


def fold_for_matching(text: str) -> str:
    """
    Produces a simplified lowercase, accent-free string for robust substring matching.
    E.g. "violación" -> "violacion"
    """
    text = text.casefold()
    text = unicodedata.normalize("NFKD", text)
    text = "".join(ch for ch in text if not unicodedata.combining(ch))
    return text


def contains_hard_fail(text: str) -> bool:
    t = fold_for_matching(text)
    return any(fold_for_matching(phrase) in t for phrase in HARD_FAIL_PHRASES)


def run_moderation(client: OpenAI, text: str) -> bool:
    """
    Returns True if moderation flags the content.
    Moderation supports Spanish; no special handling needed.
    """
    resp = client.moderations.create(
        model=MODERATION_MODEL,
        input=text
    )
    return bool(resp.results[0].flagged)


def build_pixel_art_prompt(subject: str) -> str:
    subject = normalize_text(subject)

    # Positive constraints first; minimal negatives only for common failure modes.
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

SUBJECT (from Spanish transcription; interpret literally but keep it kid-safe):
{subject}
""".strip()


def generate_image(client: OpenAI, prompt: str, out_path: str) -> None:
    """
    Uses the Images API for gpt-image-1.
    moderation="auto" enforces standard safety filtering at generation time.
    """
    img = client.images.generate(
        model=IMAGE_MODEL,
        prompt=prompt,
        size="1024x1024",
        moderation="auto",
        output_format="png",
        quality="auto",
        n=1,
    )

    b64 = img.data[0].b64_json
    with open(out_path, "wb") as f:
        f.write(base64.b64decode(b64))


# ---- Main ----

def main() -> int:
    if len(sys.argv) < 3:
        print("Usage: python make_prompt.py transcription.txt out.png [prompt.txt]")
        return 1

    transcription_file = sys.argv[1]
    out_image_file = sys.argv[2]
    out_prompt_file: Optional[str] = sys.argv[3] if len(sys.argv) >= 4 else None

    try:
        with open(transcription_file, "r", encoding="utf-8") as f:
            transcription = normalize_text(f.read())

        if not transcription:
            print("Error: Transcription is empty.")
            return 1

        # Free local guard (optional; you can delete this block if you want ONLY moderation)
        if contains_hard_fail(transcription):
            print("flagged content, prompt cancelled")
            return 1

        client = OpenAI()

        # Moderation call (paid, but small). If flagged -> cancel.
        if run_moderation(client, transcription):
            print("flagged content, prompt cancelled")
            return 1

        prompt = build_pixel_art_prompt(transcription)

        if out_prompt_file:
            with open(out_prompt_file, "w", encoding="utf-8") as f:
                f.write(prompt)

        generate_image(client, prompt, out_image_file)

        print("OK")
        if out_prompt_file:
            print(f"Prompt saved to: {out_prompt_file}")
        print(f"Image saved to: {out_image_file}")
        return 0

    except Exception as e:
        print(f"Error: {e}")
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
