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
#input = input[:5*fs]

#input = signal.sosfilt(sos, input)

fc = 13000
modulation_index = 1    # dies mal der maximalen im Signal vorkommenden Frequenz gibt die maximale Frequenzabweichung

duration = len(input) / fs  # Länge des Signals in Sekunden
t = np.linspace(0, duration, len(input), endpoint=False)  # Zeitvektor


phi = 2 * np.pi * fc * t + modulation_index * np.cumsum(input)  # wenn Ableitung hiervon je unter 0 fällt gibt es einen Fehler, bei modulation_index > 1.3, NVM
fm_signal = np.sin(phi)


### full version
analytic_signal = signal.hilbert(fm_signal)  # Hilbert-Transformation
fReal_part = np.real(analytic_signal)
fImag_part = np.imag(analytic_signal)

fDePhi = np.arctan2(fImag_part, fReal_part)
fUnwrapPhi = np.unwrap(fDePhi)
fDiffPhi = np.gradient(fUnwrapPhi, t)
fScaledPhi = fDiffPhi - 2*np.pi*fc

fullOutput = fScaledPhi
fullOutput /= np.max(np.abs(fullOutput))

### split version
pLength = 2048
parts = len(fm_signal) // pLength
output = np.zeros(parts * pLength)

outDuration = len(output) / fs  # Länge des Signals in Sekunden
outT = np.linspace(0, outDuration, len(output), endpoint=False)  # Zeitvektor

for i in range(parts):
    currentPart = fm_signal[i*pLength:(i+1)*pLength]
    
    analytic_signal = signal.hilbert(currentPart)  # Hilbert-Transformation
    real_part = np.real(analytic_signal)
    imag_part = np.imag(analytic_signal)

    #plt.plot(range(pLength), np.imag(analytic_signal2), range(pLength), np.imag(analytic_signal)[pLength//2: 3*pLength//2], range(pLength), np.imag(analytic_signal3)[pLength//2: 3*pLength//2], marker=".")
    #plt.title("imag_part TEST")
    #plt.show()

    dePhi = np.arctan2(imag_part, real_part)
    unwrapPhi = np.unwrap(dePhi)
    diffPhi = np.gradient(unwrapPhi, t[i*pLength:(i+1)*pLength])
    scaledPhi = diffPhi - 2*np.pi*fc
    output[i*pLength:(i+1)*pLength] = scaledPhi

output /= np.max(np.abs(output))

outputZero = np.zeros(parts * pLength)
for i in range(parts):
    currentPart = fm_signal[i*pLength:(i+1)*pLength]

    padded = np.pad(currentPart, (pLength//2, pLength//2))
    analytic_signal = signal.hilbert(padded)

    real_part = np.real(analytic_signal)[pLength//2: 3*pLength//2]
    imag_part = np.imag(analytic_signal)[pLength//2: 3*pLength//2]
    dePhi = np.arctan2(imag_part, real_part)
    unwrapPhi = np.unwrap(dePhi)
    diffPhi = np.gradient(unwrapPhi, t[i*pLength:(i+1)*pLength])
    scaledPhi = diffPhi - 2*np.pi*fc
    outputZero[i*pLength:(i+1)*pLength] = scaledPhi

outputZero /= np.max(np.abs(outputZero))


outputReflect = np.zeros(parts * pLength)
for i in range(parts):
    currentPart = fm_signal[i*pLength:(i+1)*pLength]

    padded = np.pad(currentPart, (pLength//2, pLength//2), mode='reflect')
    analytic_signal = signal.hilbert(padded)

    
    real_part = np.real(analytic_signal)[pLength//2: 3*pLength//2]
    imag_part = np.imag(analytic_signal)[pLength//2: 3*pLength//2]
    dePhi = np.arctan2(imag_part, real_part)
    unwrapPhi = np.unwrap(dePhi)
    diffPhi = np.gradient(unwrapPhi, t[i*pLength:(i+1)*pLength])
    scaledPhi = diffPhi - 2*np.pi*fc
    outputReflect[i*pLength:(i+1)*pLength] = scaledPhi

outputReflect /= np.max(np.abs(outputReflect))


outputEdge = np.zeros(parts * pLength)
for i in range(parts):
    currentPart = fm_signal[i*pLength:(i+1)*pLength]

    padded = np.pad(currentPart, (pLength//2, pLength//2), mode='edge')
    analytic_signal = signal.hilbert(padded)

    real_part = np.real(analytic_signal)[pLength//2: 3*pLength//2]
    imag_part = np.imag(analytic_signal)[pLength//2: 3*pLength//2]
    dePhi = np.arctan2(imag_part, real_part)
    unwrapPhi = np.unwrap(dePhi)
    diffPhi = np.gradient(unwrapPhi, t[i*pLength:(i+1)*pLength])
    scaledPhi = diffPhi - 2*np.pi*fc
    outputEdge[i*pLength:(i+1)*pLength] = scaledPhi

outputEdge /= np.max(np.abs(outputEdge))


intOutput = np.int16(outputZero * 32767)
wavfile.write(".\\output.wav", fs, intOutput)


#plt.plot(outT, output, outT, outputZero, outT, outputReflect, outT, outputEdge, marker=".")
#plt.title("Output")
#plt.show()

plt.plot(outT, outputZero, outT, output, marker=".")
plt.title("Output")
plt.show()

plt.plot(outT, outputZero, t, fullOutput, marker=".")
plt.title("Output")
plt.show()
