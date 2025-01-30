import sounddevice as sd
import numpy as np
import soundfile as sf
import matplotlib.pyplot as plt

# Parameter für die Verarbeitung
block_size = 60000  # Blockgröße: 1 Sekunde bei 48 kHz Samplingrate
buffer_size = block_size * 10  # Größe des Ringpuffers (10 Blöcke)
carrier_freq = 7000  # Trägerfrequenz (gleicher Wert wie bei der Modulation)
input_file = "fm_audio.wav"

# FM-Demodulation
def fm_demodulate(block, carrier_freq, sr):
    # Hilbertraum: Differenz der Phasen
    analytic_signal = np.angle(block[1:] * np.conj(block[:-1]))
    demodulated_signal = np.unwrap(analytic_signal) / (2 * np.pi * carrier_freq / sr)
    return demodulated_signal

# Ringpuffer-Klasse
class RingBuffer:
    def __init__(self, size):
        self.size = size
        self.buffer = np.zeros(size, dtype='float64')
        self.start = 0
        self.end = 0
        self.full = False

    def write(self, data):
        data_size = len(data)
        if data_size > self.size:
            raise ValueError("Datenmenge überschreitet Puffergröße")

        end_space = self.size - self.end
        if data_size <= end_space:
            self.buffer[self.end:self.end + data_size] = data
        else:
            self.buffer[self.end:] = data[:end_space]
            self.buffer[:data_size - end_space] = data[end_space:]

        self.end = (self.end + data_size) % self.size
        self.full = self.full or (self.end == self.start)

    def read(self, size):
        if size > self.size:
            raise ValueError("Lesemenge überschreitet Puffergröße")

        if self.end == self.start and not self.full:
            return np.zeros(size)  # Puffer leer

        if self.full or self.end > self.start:
            available = self.end - self.start if not self.full else self.size
        else:
            available = self.size - self.start + self.end

        if size > available:
            raise ValueError("Nicht genug Daten im Puffer verfügbar")

        end_space = self.size - self.start
        if size <= end_space:
            data = self.buffer[self.start:self.start + size]
        else:
            data = np.concatenate((self.buffer[self.start:], self.buffer[:size - end_space]))

        self.start = (self.start + size) % self.size
        self.full = False
        return data

# Echtzeit-Streaming-Funktion mit Ringpuffer
def stream_audio_with_ringbuffer():
    ring_buffer = RingBuffer(buffer_size)

    # Datei öffnen und Metadaten auslesen
    with sf.SoundFile(input_file, mode='r') as file:
        sr = file.samplerate  # Samplingrate
        duration = file.frames / sr  # Gesamtdauer des Signals in Sekunden
        print(f"Eingabedatei: {input_file}, Samplerate: {sr}, Dauer: {duration:.2f} Sekunden")

        # Blockweise Verarbeitung und Echtzeit-Ausgabe
        for i, block in enumerate(file.blocks(block_size, dtype='float64')):
            print(f"Verarbeite Block {i + 1}...")

            # Normierung
            block = block / np.max(np.abs(block))

            # Daten in den Ringpuffer schreiben
            ring_buffer.write(block)

            # Demodulierte Daten aus dem Puffer lesen
            if i >= 10:  # Warten, bis der Puffer ausreichend gefüllt ist
                buffered_data = ring_buffer.read(block_size)
                demodulated = fm_demodulate(buffered_data, carrier_freq, sr)
                demodulated = demodulated / np.max(np.abs(demodulated))  # Normalisieren

                # Demoduliertes Signal ausgeben
                sd.play(demodulated.astype('float32'), sr)

                # Warten, bis das aktuelle Block abgespielt wurde
                sd.wait()

# Start der Echtzeitverarbeitung
stream_audio_with_ringbuffer()
