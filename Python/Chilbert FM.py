import numpy as np
import matplotlib.pyplot as plt
from scipy import integrate, signal
from scipy.io import wavfile

# prepared filters
sos = signal.butter(60, 10000, 'lowpass', fs=48000, output='sos')


# input and modulation
fs, input = wavfile.read(".\\der_mann_der_vom_flugzeug_sprang.wav")

if input.ndim > 1:
    input = input.mean(axis=1)

input = input / np.max(np.abs(input))  # Normalisieren auf -1 bis 1
#input -= np.mean(input)  # DC-Offset entfernen

#input = signal.sosfilt(sos, input)

fc = 13000
modulation_index = 1    # dies mal der maximalen im Signal vorkommenden Frequenz gibt die maximale Frequenzabweichung

duration = len(input) / fs  # Länge des Signals in Sekunden
t = np.linspace(0, duration, len(input), endpoint=False)  # Zeitvektor


phi = 2 * np.pi * fc * t + modulation_index * np.cumsum(input)  # wenn Ableitung hiervon je unter 0 fällt gibt es einen Fehler, bei modulation_index > 1.3, NVM
fm_signal = np.sin(phi)

# fm_signal = np.int16(fm_signal / np.max(np.abs(fm_signal)) * 32767)


def demodulate(signalVector, timeVector):
    analytic_signal = signal.hilbert(signalVector)  # Hilbert-Transformation
    real_part = np.real(analytic_signal)
    imag_part = np.imag(analytic_signal)

    dePhi = np.arctan2(imag_part, real_part)
    unwrapPhi = np.unwrap(dePhi)
    diffPhi = np.gradient(unwrapPhi, timeVector)
    scaledPhi = diffPhi - 2*np.pi*fc
    return scaledPhi


### full version
analytic_signal = signal.hilbert(fm_signal)  # Hilbert-Transformation
real_part = np.real(analytic_signal)
imag_part = np.imag(analytic_signal)

dePhi = np.arctan2(imag_part, real_part)
unwrapPhi = np.unwrap(dePhi)
diffPhi = np.gradient(unwrapPhi, t)
scaledPhi = diffPhi - 2*np.pi*fc

fullOutput = scaledPhi
fullOutput /= np.max(np.abs(fullOutput))


### split version
pLength = 2048
parts = len(fm_signal) // pLength
output = np.empty(parts * pLength)

for i in range(parts):
    currentPart = fm_signal[i*pLength:(i+1)*pLength]

    output[i*pLength:(i+1)*pLength] = demodulate(currentPart, t[i*pLength:(i+1)*pLength])

# output filtering and time vector creation for plotting
#output = signal.sosfilt(sos, output)
output /= np.max(np.abs(output))

outDuration = len(output) / fs  # Länge des Signals in Sekunden
outT = np.linspace(0, outDuration, len(output), endpoint=False)  # Zeitvektor


### special version
specialOutput = np.zeros(parts * pLength)
halfPLength = pLength//2
quarterPLength = halfPLength//2

for i in range(2*parts-1):
    currentPart = fm_signal[i*halfPLength:(i+2)*halfPLength]

    specialOutput[i*halfPLength+quarterPLength:(i+2)*halfPLength-quarterPLength] = demodulate(currentPart, t[i*halfPLength:(i+2)*halfPLength])[quarterPLength:pLength-quarterPLength]

specialOutput /= np.max(np.abs(specialOutput))


intOutput = np.int16(output * 32767)
wavfile.write(".\\output.wav", fs, intOutput)


plt.plot(t, fullOutput, outT, output, outT, specialOutput, marker=".")
plt.title("Output")
plt.show()

plt.plot(outT, specialOutput, t, fullOutput, marker=".")
plt.title("Output")
plt.show()

#plt.plot(outT, output, marker=".")
#plt.title("Output")
#plt.show()

#gradientPhi = np.gradient(phi, t)
#print("Minimum:", np.min(gradientPhi))
#plt.plot(t, gradientPhi, marker=".")
#plt.title("gradient of phi")
#plt.show()

#plt.plot(t, phi, t, unwrapPhi, marker=".")
#plt.title("phi")
#plt.show()

#plt.plot(t, fm_signal, marker=".")
#plt.title("fm_signal")
#plt.show()
#
#plt.plot(t, real_part, marker=".")
#plt.title("real_part")
#plt.show()
#
#plt.plot(t, imag_part, marker=".")
#plt.title("imag_part")
#plt.show()

#plt.plot(t, scaledPhi, marker=".")
#plt.title("scaledPhi")
#plt.show()
#
#plt.plot(t, scaledPhi2, marker=".")
#plt.title("scaledPhi2")
#plt.show()
#
#plt.plot(output, marker=".")
#plt.title("output")
#plt.show()