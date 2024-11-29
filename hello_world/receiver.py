import numpy as np
from scipy.io import wavfile
from scipy.fft import fft
import matplotlib.pyplot as plt


# Parameters
Fs = 96000
Fc = 6000
SampleDivider = Fs//Fc
Quantisierungsstufen = 7
offset = Fc

audio = wavfile.read('./audio/modulated_audio.wav')[1]
audio = audio / np.max(np.abs(audio))


demodulated = np.zeros(len(audio))

demodulated = (demodulated - (Quantisierungsstufen/2-0.5)) / (Quantisierungsstufen/2)


# Write the demodulated audio signal to a file
wavfile.write('./audio/demodulated_audio.wav', Fc, demodulated)


# Plot the audio signal
plt.figure()
plt.plot(demodulated)
plt.title('Demodulated Audio Signal')
plt.xlabel('Sample')
plt.ylabel('Amplitude')
plt.grid()
plt.show()

