from scipy.io import wavfile
import numpy as np
from scipy.io.wavfile import write
import matplotlib.pyplot as plt
import soundfile as sf
from scipy.signal import butter, lfilter

# Parameter für die Verarbeitung
block_size = 48000  # Blockgröße: 1 Sekunde bei 48 kHz Samplingrate
cutoff_frequency = 10000  # Grenzfrequenz des Tiefpassfilters in Hz
filter_order = 10  # Ordnung des Tiefpassfilters

# Eingabedatei (FM-Signal)
input_file = "fm_audio.wav"
# Ausgabedatei (demoduliertes und gefiltertes Signal)
output_file = "C:\\Users\\ludwi\\Documents\\H_KA_Studium\\0IB6_Digitale_Signalprozessoren\\DSP_FM_Demod_VSC\\Python\\demodulated_audio.wav"

# FM-Demodulation
def fm_demodulate(block, carrier_freq, sr):
    # Hilbertraum: Differenz der Phasen
    analytic_signal = np.angle(block[1:] * np.conj(block[:-1]))
    demodulated_signal = np.unwrap(analytic_signal) / (2 * np.pi * carrier_freq / sr)
    return demodulated_signal

# Datei öffnen und Metadaten auslesen
with sf.SoundFile(input_file, mode='r') as file:
    sr = file.samplerate  # Samplingrate
    duration = file.frames / sr  # Gesamtdauer des Signals in Sekunden
    print(f"Eingabedatei: {input_file}, Samplerate: {sr}, Dauer: {duration:.2f} Sekunden")
    
    # Ausgabedatei erstellen
    with sf.SoundFile(output_file, mode='w', samplerate=sr, channels=1, subtype='PCM_16') as output:
        carrier_freq = 7000  # Trägerfrequenz (gleicher Wert wie bei der Modulation)

        prev_block = None  # Initialisierung für die Nahtstellen zwischen Blöcken
        
        # Blockweise Verarbeitung
        for i, block in enumerate(file.blocks(block_size, dtype='float64')):
            print(f"Verarbeite Block {i + 1} von {int(duration * sr / block_size)}...")

            # Normierung
            block = block / np.max(np.abs(block))
            
            # Nahtstelle zwischen Blöcken behandeln
            if prev_block is not None:
                block = np.concatenate((prev_block[-1:], block))

            # FM-Demodulation
            demodulated = fm_demodulate(block, carrier_freq, sr)

            # Signal auf ursprüngliche Samplingrate zurücksetzen
            demodulated = demodulated / np.max(np.abs(demodulated))  # Normalisieren auf -1 bis 1
            output.write(demodulated.astype('float32'))  # Block in Datei schreiben
            
            # Speichere den aktuellen Block für die Nahtstelle
            prev_block = block

    print(f"Demoduliertes Signal wurde gespeichert als '{output_file}'.")

# Visualisierung
with sf.SoundFile(output_file, mode='r') as demod_file:
    demodulated_signal = demod_file.read()
    time = np.linspace(0, len(demodulated_signal) / sr, len(demodulated_signal))

    plt.figure(figsize=(10, 6))
    plt.plot(time, demodulated_signal, label="Demoduliertes Signal")
    plt.title("Demoduliertes Signal (Zeitraum)")
    plt.xlabel("Zeit [s]")
    plt.ylabel("Amplitude")
    plt.legend()
    plt.show()

# Spektralanalyse (FFT)
N = len(demodulated_signal)
frequencies = np.fft.fftfreq(N, 1/sr)  # Frequenzen für die X-Achse
spectrum = np.fft.fft(demodulated_signal)  # FFT des demodulierten Signals
spectrum_magnitude = np.abs(spectrum)  # Betrag des Spektrums

# Plot des Spektrums
plt.figure(figsize=(10, 6))
plt.plot(frequencies[:N//2], spectrum_magnitude[:N//2])  # Nur positive Frequenzen anzeigen
plt.title("Spektrum des demodulierten Signals")
plt.xlabel("Frequenz [Hz]")
plt.ylabel("Magnitude")
plt.xlim(0, sr / 2)  # Frequenzen bis zur Nyquist-Frequenz
plt.grid(True)
plt.show()

