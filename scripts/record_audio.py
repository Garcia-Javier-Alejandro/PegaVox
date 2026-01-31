"""
record_audio.py

Milestone 1: Record Audio from Microphone
- Captures a short audio clip from the PC microphone and saves it as a WAV file.
- Usage: python record_audio.py [output.wav] [duration_seconds]
"""
import sys
import sounddevice as sd
import numpy as np
import wave

# Default parameters
OUTPUT_FILE = sys.argv[1] if len(sys.argv) > 1 else "output.wav"
DURATION = float(sys.argv[2]) if len(sys.argv) > 2 else 5.0  # seconds
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
