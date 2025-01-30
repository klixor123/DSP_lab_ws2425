from scipy.io import wavfile
import numpy as np
from scipy.io.wavfile import write
import matplotlib.pyplot as plt

# WAV-Datei laden
sr, mod_signal = wavfile.read(".\\der_mann_der_vom_flugzeug_sprang.wav")  # Samplingrate und Daten

# Falls Stereo, in Mono umwandeln
if mod_signal.ndim > 1:
    mod_signal = mod_signal.mean(axis=1)

# Normierung und DC-Offset entfernen
mod_signal = mod_signal / np.max(np.abs(mod_signal))  # Normalisieren auf -1 bis 1
mod_signal -= np.mean(mod_signal)  # DC-Offset entfernen

# Parameter für Trägersignal
carrier_freq = 13000  # Trägerfrequenz in Hz
duration = len(mod_signal) / sr  # Länge des Signals in Sekunden
t = np.linspace(0, duration, len(mod_signal), endpoint=False)  # Zeitvektor
modulation_index = 1 # Modulationstiefe

# Frequenzmodulation
fm_signal = np.sin(2 * np.pi * carrier_freq * t + modulation_index * np.cumsum(mod_signal)) #kf modulationsindex

# Normierung für 16-Bit Integer
fm_signal = np.int16(fm_signal / np.max(np.abs(fm_signal)) * 32767) 

# Speichern als WAV mit 48 kHz, Mono, 16 Bit
wavfile.write(".\\fm_audio.wav", 48000, fm_signal)

print("Frequenzmoduliertes Signal wurde gespeichert als 'fm_audio.wav'.")

# Zeitbereich - Darstellung
time = np.linspace(0, len(fm_signal) / sr, num=len(fm_signal))

plt.figure(figsize=(10, 6))
plt.subplot(2, 1, 1)
plt.plot(time, fm_signal)
plt.title('Zeitraum-Darstellung (Zeitbereich)')
plt.xlabel('Zeit [s]')
plt.ylabel('Amplitude')

# Frequenzbereich - Spektrum
# FFT (Fast Fourier Transform) berechnen
n = len(fm_signal)
frequencies = np.fft.fftfreq(n, 1 / sr)
fft_values = np.fft.fft(fm_signal)

# Nur positive Frequenzen anzeigen
positive_freqs = frequencies[:n // 2]
positive_fft_values = np.abs(fft_values)[:n // 2]

plt.subplot(2, 1, 2)
plt.plot(positive_freqs, positive_fft_values)
plt.title('Spektrum (Frequenzbereich)')
plt.xlabel('Frequenz [Hz]')
plt.ylabel('Amplitude')
plt.xlim(0, 25000)  # Beispiel: bis 10 kHz, um das FM-Spektrum zu zeigen

plt.tight_layout()
plt.show()