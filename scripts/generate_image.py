"""
generate_image.py

Milestone 4: Generate DALL·E 2 Image from Prompt
- Reads a prompt from prompt.txt in the scripts directory, sends it to OpenAI DALL·E 2, and saves the generated image as dalle_output.png in scripts.
- Requires: openai, requests, Pillow

How to run test:
    python generate_image.py prompt.txt dalle_output.png
    # All files are expected in /scripts; use only filenames, not paths.
"""
import sys
import requests
import os
from PIL import Image
from io import BytesIO

OPENAI_API_KEY = os.getenv("OPENAI_API_KEY")

if not OPENAI_API_KEY:
    print("Error: Set your OpenAI API key in the OPENAI_API_KEY environment variable.")
    sys.exit(1)

if len(sys.argv) < 3:
    print("Usage: python generate_image.py prompt.txt output.png")
    sys.exit(1)

PROMPT_FILE = sys.argv[1]
OUTPUT_IMAGE = sys.argv[2]

try:
    with open(PROMPT_FILE, 'r', encoding='utf-8') as f:
        prompt = f.read().strip()

    print(f"Sending prompt to DALL·E 2: {prompt}")
    url = "https://api.openai.com/v1/images/generations"
    headers = {"Authorization": f"Bearer {OPENAI_API_KEY}"}
    data = {
        "model": "dall-e-2",
        "prompt": prompt,
        "n": 1,
        "size": "512x512"
    }
    response = requests.post(url, headers=headers, json=data)
    response.raise_for_status()
    result = response.json()
    image_url = result["data"][0]["url"]

    print(f"Downloading image from: {image_url}")
    img_response = requests.get(image_url)
    img_response.raise_for_status()
    img = Image.open(BytesIO(img_response.content))
    # Resize to 384x384 for printer compatibility
    img = img.resize((384, 384), Image.LANCZOS)
    img.save(OUTPUT_IMAGE)
    print(f"Image saved as {OUTPUT_IMAGE}")
except Exception as e:
    print(f"Error: {e}")
    if hasattr(e, 'response') and e.response is not None:
        print(e.response.text)
    sys.exit(1)
