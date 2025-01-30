from scipy.io import wavfile
import numpy as np
import matplotlib.pyplot as plt
import soundfile as sf
from scipy.signal import butter, lfilter, hilbert

# Parameter für die Verarbeitung
cutoff_frequency = 20000  # Grenzfrequenz des Tiefpassfilters in Hz
filter_order = 10  # Ordnung des Tiefpassfilters

# Eingabedatei (FM-Signal)
input_file = "fm_audio.wav"
# Ausgabedatei (demoduliertes und gefiltertes Signal)
output_file = ".\\demodulated_audio_filtered.wav"

# Tiefpassfilter-Funktion
def butter_lowpass_filter(data, cutoff, fs, order):
    nyquist = 0.5 * fs
    normal_cutoff = cutoff / nyquist
    b, a = butter(order, normal_cutoff, btype='low', analog=False)
    return lfilter(b, a, data)

# FM-Demodulation
def fm_demodulate_access_phase(signal, carrier_freq, sr):
    """
    FM-Demodulation basierend auf der Methode "Access Phase and Differentiate".
    """
    # Analytisches Signal berechnen
    analytic_signal = hilbert(signal)  # Hilbert-Transformation
    real_part = np.real(analytic_signal)
    imag_part = np.imag(analytic_signal)

    # Phase berechnen
    phase = np.arctan2(imag_part, real_part)

    # Differentiation der Phase
    derivative_phase = np.diff(phase)  # Numerische Differentiation

    # Phasensprünge korrigieren
    demodulated = np.unwrap(derivative_phase)  # Entfaltung der Phase

    # Normierung
    demodulated /= (2 * np.pi * carrier_freq / sr)  # Auf Basisband skalieren

    return demodulated

# Datei öffnen und Metadaten auslesen
with sf.SoundFile(input_file, mode='r') as file:
    sr = file.samplerate  # Samplingrate
    duration = file.frames / sr  # Gesamtdauer des Signals in Sekunden
    print(f"Eingabedatei: {input_file}, Samplerate: {sr}, Dauer: {duration:.2f} Sekunden")
    
    # Komplettes Signal laden
    fm_signal = file.read(dtype='float64')
    print("Signal geladen. Verarbeitung startet...")

# FM-Demodulation
carrier_freq = 13000  # Trägerfrequenz
demodulated_signal = fm_demodulate_access_phase(fm_signal, carrier_freq, sr)

# Tiefpassfilter auf demoduliertes Signal anwenden
demodulated_filtered = butter_lowpass_filter(demodulated_signal, cutoff_frequency, sr, filter_order)

# Gleichanteil entfernen und normalisieren
demodulated_filtered -= np.mean(demodulated_filtered)  # DC-Offset entfernen
demodulated_filtered /= np.max(np.abs(demodulated_filtered))  # Normalisieren auf -1 bis 1

# Signal speichern
sf.write(output_file, demodulated_filtered.astype('float32'), sr)
print(f"Demoduliertes und gefiltertes Signal wurde gespeichert als '{output_file}'.")

# Visualisierung
time = np.linspace(0, len(demodulated_filtered) / sr, len(demodulated_filtered))

# Zeitbereich
plt.figure(figsize=(12, 8))
plt.subplot(2, 1, 1)
plt.plot(time, demodulated_filtered, label="Demoduliertes Signal (gefiltert)")
plt.title("Demoduliertes und gefiltertes Signal (Zeitraum)")
plt.xlabel("Zeit [s]")
plt.ylabel("Amplitude")
plt.legend()

# Frequenzbereich (Spektrum)
n = len(demodulated_filtered)
freqs = np.fft.rfftfreq(n, 1 / sr)  # Frequenzachsenwerte
fft_values = np.abs(np.fft.rfft(demodulated_filtered))  # FFT berechnen

plt.subplot(2, 1, 2)
plt.plot(freqs, fft_values, label="Spektrum (nach Demodulation und Filterung)")
plt.title("Spektrum des demodulierten Signals")
plt.xlabel("Frequenz [Hz]")
plt.ylabel("Amplitude")
plt.legend()
plt.tight_layout()
plt.show()

