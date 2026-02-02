"""
record_audio.py

Milestone 1: Record Audio from Microphone
- Captures a short audio clip from the PC microphone and saves it as a WAV file in the scripts directory.
- Requires: sounddevice, numpy

How to run test:
    # From scripts directory
    python record_audio.py output.wav 5
    # All files are expected in /scripts; use only filenames, not paths.
    # Duration must be 30 seconds or less
"""
import sys
import sounddevice as sd
import numpy as np
import wave

# Default parameters

DFAULT_OUTPUT = "output.wav"
DEFAULT_PATH = "./"  # Save in /scripts
OUTPUT_FILE = sys.argv[1] if len(sys.argv) > 1 else DEFAULT_PATH + DFAULT_OUTPUT
DURATION = float(sys.argv[2]) if len(sys.argv) > 2 else 5.0  # seconds
if DURATION > 30.0:
    print("Error: Maximum recording duration is 30 seconds.")
    sys.exit(1)
SAMPLE_RATE = 16000  # 16 kHz, mono

import time

print(f"Recording {DURATION} seconds of audio to '{OUTPUT_FILE}'...")
try:
    audio = sd.rec(int(DURATION * SAMPLE_RATE), samplerate=SAMPLE_RATE, channels=1, dtype='int16')
    for remaining in range(int(DURATION), 0, -1):
        print(f"...{remaining} second(s) remaining...")
        time.sleep(1)
    sd.wait()
    audio = np.squeeze(audio)

    with wave.open(OUTPUT_FILE, 'wb') as wf:
        wf.setnchannels(1)
        wf.setsampwidth(2)  # 16 bits
        wf.setframerate(SAMPLE_RATE)
        wf.writeframes(audio.tobytes())

    print(f"Done. File saved: {OUTPUT_FILE}")
except Exception as e:
    print(f"Error: {e}")
    sys.exit(1)
