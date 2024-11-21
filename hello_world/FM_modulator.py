import numpy as np
from scipy.io import wavfile
import matplotlib.pyplot as plt

# Parameters
Fs = 96000
Fc = 6000
SampleDivider = Fs//Fc
Quantisierungsstufen = 7
offset = Fc
# def FM_modulator(audio, Fs, Fc, beta):
#     t = np.arange(0, len(audio)/Fs, 1/Fs)
#     carrier = np.cos(2*np.pi*Fc*t)
#     audio = np.interp(np.arange(0, len(audio), Fc/Fs), np.arange(0, len(audio)), audio)
#     modulated = np.cos(2*np.pi*Fc*t + 2*np.pi*beta*audio)
#     return modulated

def quant(audio):
    audio = np.clip(audio, -0.9999, 0.9999)     # Clip the audio signal to 
    Delta = 2 / Quantisierungsstufen
    quantized = (np.floor((audio+1)/Delta) + 0.5) * Delta - 1
    quantized = quantized * Quantisierungsstufen/2 + Quantisierungsstufen/2 - 0.5
    return quantized

def FM_modulator(signal):
    t = np.arange(0, len(signal)/Fc, 1/Fs)
    signal = np.repeat(signal, SampleDivider)
    t = t[:len(signal)]                                 # Remove the last element to make the length of t and signal equal
    modulated = np.cos(2*np.pi*t*(Fc*signal + offset))
    return modulated

# Read the audio file
Fs_audio, audio = wavfile.read('./audio/audio.wav')
audio = audio / np.max(np.abs(audio))

# Only use every SampleDivider*th sample
audio = audio[::Fs_audio//Fc]

# Quantize the audio signal
quantized = quant(audio)

# Modulate the quantized audio signal
modulated = FM_modulator(quantized)

# Write the quantized audio signal to a file
wavfile.write('./audio/quantized_audio.wav', Fs, modulated)

# Plot the quantized audio signal
plt.figure()
plt.plot(quantized)
plt.title('Quantized Audio Signal')
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