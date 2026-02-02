"""
pegavox_demo.py

Milestone 5: End-to-End Demo Script
- Runs the full workflow: record audio, transcribe, generate prompt, and create image.
- All files are handled in the scripts directory.

How to run test:
    python pegavox_demo.py
"""
import subprocess
import sys
import os

# Filenames
AUDIO_FILE = "output.wav"
TRANSCRIPTION_FILE = "transcription.txt"
PROMPT_FILE = "prompt.txt"
IMAGE_FILE = "dalle_output.png"

SCRIPTS_DIR = os.path.dirname(os.path.abspath(__file__))

try:
    print("Step 1: Recording audio (15 seconds)...")
    subprocess.run([sys.executable, "record_audio.py", AUDIO_FILE, "15"], cwd=SCRIPTS_DIR, check=True)

    print("Step 2: Transcribing audio...")
    subprocess.run([sys.executable, "transcribe_audio.py", AUDIO_FILE, TRANSCRIPTION_FILE], cwd=SCRIPTS_DIR, check=True)

    print("Step 3: Generating prompt...")
    subprocess.run([sys.executable, "make_prompt.py", TRANSCRIPTION_FILE, PROMPT_FILE], cwd=SCRIPTS_DIR, check=True)

    print("Step 4: Generating image...")
    subprocess.run([sys.executable, "generate_image.py", PROMPT_FILE, IMAGE_FILE], cwd=SCRIPTS_DIR, check=True)

    print(f"Demo complete! Image saved as {IMAGE_FILE}")
except subprocess.CalledProcessError as e:
    print(f"Error in step: {e}")
    sys.exit(1)
except Exception as e:
    print(f"Unexpected error: {e}")
    sys.exit(1)
