import numpy as np
from scipy.io import wavfile
import matplotlib.pyplot as plt

# Parameters
Fs = 48000
Fc = 13000
kf = 10000 # Proportional to the frequency deviation kf * Amplitude = delta_f 

t = np.arange(0, 1, 1/Fs)

sine = np.sin(2 * np.pi * 10 * t)
integral = np.cumsum(sine, dtype=float) / Fs


# Read the audio file
Fs_audio, audio = wavfile.read('./audio/audio.wav')
audio = audio / np.max(np.abs(audio))
t_audio = np.arange(0, len(audio))/Fs_audio

integral = np.cumsum(audio, dtype=float) / Fs_audio


modulated = np.zeros(len(audio))
for i in range(len(audio)):
    modulated[i] = np.cos(2 * np.pi * (Fc*t_audio[i] + kf * integral[i]))


# Quantize the audio signal
#quantized = quant(audio)

# Modulate the quantized audio signal
#modulated = FM_modulator(quantized)

# Write the quantized audio signal to a file
wavfile.write('./audio/modulated_audio.wav', Fs_audio, modulated)

# Plot the quantized audio signal
plt.figure()
plt.plot(audio)
plt.title('Audio Signal')
plt.xlabel('Sample')
plt.ylabel('Amplitude')
plt.grid()
plt.show()

# Plot the modulated audio signal
plt.figure()
plt.plot(modulated)
plt.title('Modulated Audio Signal') 
plt.xlabel('Sample')
plt.ylabel('Amplitude')
plt.grid()
plt.show()