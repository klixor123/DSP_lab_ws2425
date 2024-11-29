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

audio = wavfile.read('./audio/quantized_audio.wav')[1]
audio = audio / np.max(np.abs(audio))

def FM_demodulator(signal):
    demodulated = np.zeros(len(signal)//SampleDivider)    
    for x in range(0, len(signal), SampleDivider):
        freqs = fft(signal[x:x+SampleDivider])
        freqs = np.abs(freqs)
        freqsmax = np.argmax(freqs) - 1 # -2 because fft arrays start at 1 and we skipped the first 2 kHz
        demodulated[int(x/SampleDivider)] = freqsmax
    return demodulated

demodulated = FM_demodulator(audio)

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

