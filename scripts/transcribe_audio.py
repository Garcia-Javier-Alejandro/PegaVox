"""
transcribe_audio.py

Milestone 2: Transcribe Audio to Text (Speech-to-Text)
- Uses OpenAI Whisper API to transcribe a WAV file to text.
- Requires: openai, requests

How to run test:
    # From scripts directory
    python transcribe_audio.py output.wav transcription.txt
    # All files are expected in /scripts; use only filenames, not paths.
"""
import sys
import requests
import os

# Use the OPENAI_API_KEY environment variable only (do not hardcode keys)
OPENAI_API_KEY = os.getenv("OPENAI_API_KEY")

if not OPENAI_API_KEY:
    print("Error: Set your OpenAI API key in the OPENAI_API_KEY environment variable.")
    sys.exit(1)

if len(sys.argv) < 2:
    print("Usage: python transcribe_audio.py input.wav [output.txt]")
    sys.exit(1)

AUDIO_FILE = sys.argv[1]
OUTPUT_FILE = sys.argv[2] if len(sys.argv) > 2 else None

url = "https://api.openai.com/v1/audio/transcriptions"
headers = {"Authorization": f"Bearer {OPENAI_API_KEY}"}
files = {"file": (AUDIO_FILE, open(AUDIO_FILE, "rb"), "audio/wav")}
data = {"model": "whisper-1", "language": "es"}

print(f"Transcribing '{AUDIO_FILE}'...")

try:
    response = requests.post(url, headers=headers, files=files, data=data)
    response.raise_for_status()
    result = response.json()
    print("Transcription:")
    print(result["text"])
    if OUTPUT_FILE:
        with open(OUTPUT_FILE, 'w', encoding='utf-8') as f:
            f.write(result["text"])
        print(f"Transcription written to {OUTPUT_FILE}")
except Exception as e:
    print(f"Error: {e}")
    if hasattr(e, 'response') and e.response is not None:
        print(e.response.text)
    sys.exit(1)
