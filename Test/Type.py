import wave
import numpy as np

# Open the WAV file
with wave.open("output.wav", "rb") as wav_file:
    # Get basic information
    n_channels = wav_file.getnchannels()
    sample_width = wav_file.getsampwidth()
    framerate = wav_file.getframerate()
    n_frames = wav_file.getnframes()
    
    print("Number of Channels:", n_channels)
    print("Sample Width (bytes):", sample_width)
    print("Frame Rate (samples per second):", framerate)
    print("Number of Frames:", n_frames)
    
    # Read frames and convert to a numpy array
    frames = wav_file.readframes(n_frames)
    audio_data = np.frombuffer(frames, dtype=np.int16)
    
    # Reshape based on number of channels
    if n_channels > 1:
        audio_data = audio_data.reshape(-1, n_channels)
    
    print("Audio Data Shape:", audio_data.shape)

# This will print the shape and other properties of the WAV file
