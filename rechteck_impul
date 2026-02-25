import numpy as np
import matplotlib.pyplot as plt

# ---------------------------------------------------------
# 1) Physikalische Konstanten
# ---------------------------------------------------------
c = 299792458.0           # Lichtgeschwindigkeit (m/s)
eps0 = 8.8541878128e-12   # elektrische Feldkonstante

# ---------------------------------------------------------
# 2) Simulationseinstellungen
# ---------------------------------------------------------
fs = 200e6                # Samplingfrequenz (Hz)
dt = 1/fs
T = 1e-5                  # Gesamtsimulationszeit (s)
N = int(T * fs)
t = np.arange(N) * dt

# ---------------------------------------------------------
# 3) Rechteckpuls erzeugen
# ---------------------------------------------------------
tau = 50e-9               # Pulsbreite (50 ns)
t0 = 1e-6                 # Pulsmitte

pulse = np.where((t >= t0 - tau/2) & (t <= t0 + tau/2), 1.0, 0.0)

# ---------------------------------------------------------
# 4) FFT des Pulses
# ---------------------------------------------------------
Nfft = N
S = np.fft.fft(pulse, Nfft)
f = np.fft.fftfreq(Nfft, dt)
f_shift = np.fft.fftshift(f)

# ---------------------------------------------------------
# 5) Zwei-Strahl-Geometrie
# ---------------------------------------------------------
h_s = 1.5   # Senderhöhe (m)
h_r = 1.0   # Empfängerhöhe (m)
d = 50.0    # horizontale Distanz (m)

L_direct = np.sqrt(d**2 + (h_s - h_r)**2)
L_reflected = np.sqrt(d**2 + (h_s + h_r)**2)

tau_direct = L_direct / c
tau_reflected = L_reflected / c
delta_tau = tau_reflected - tau_direct

print(f"Direct path: {L_direct:.3f} m, delay: {tau_direct*1e6:.3f} µs")
print(f"Reflected path: {L_reflected:.3f} m, delay: {tau_reflected*1e6:.3f} µs")
print(f"Delay difference: {delta_tau*1e9:.3f} ns")

# ---------------------------------------------------------
# 6) Bodenparameter (komplexe Permittivität)
# ---------------------------------------------------------
eps_r = 15.0       # reale Permittivität
sigma = 0.01       # Leitfähigkeit (S/m)

omega = 2 * np.pi * np.abs(f)
omega_safe = np.where(omega == 0, 1e-30, omega)

eps_complex = eps_r - 1j * sigma / (omega_safe * eps0)

# Einfallswinkel für reflektierten Strahl
theta = np.arctan2(h_s + h_r, d)
cos_theta = np.cos(theta)

# Fresnel-Koeffizient (parallel/polarisiert)
sqrt_term = np.sqrt(eps_complex - np.sin(theta)**2)
R_par = (eps_complex * cos_theta - sqrt_term) / (eps_complex * cos_theta + sqrt_term)

R_freq = R_par

# ---------------------------------------------------------
# 7) Frequenzabhängige Phase für reflektierte Welle
# ---------------------------------------------------------
phase = np.exp(-1j * 2 * np.pi * f * delta_tau)

# amplitude scaling (1/r)
A_d = 1 / L_direct
A_r = 1 / L_reflected

# kombiniertes Spektrum
S_comb = S * (A_d + A_r * R_freq * phase)

# Zeitbereichssignal zurück
s_comb = np.fft.ifft(S_comb).real

# ---------------------------------------------------------
# 8) PLOTS
# ---------------------------------------------------------

# Zeitbereich — Puls
plt.figure(figsize=(8,4))
plt.plot(t*1e6, pulse)
plt.title("Rechteckpuls (Zeitbereich)")
plt.xlabel("Zeit (µs)")
plt.ylabel("Amplitude")
plt.grid(True)
plt.show()

# Spektrum des Pulses
S_mag = 20 * np.log10(np.abs(np.fft.fftshift(S)) + 1e-20)

plt.figure(figsize=(8,4))
plt.plot(f_shift/1e6, S_mag)
plt.title("FFT des Pulses")
plt.xlabel("Frequenz (MHz)")
plt.ylabel("Magnitude (dB)")
plt.grid(True)
plt.xlim(0, fs/2/1e6)
plt.show()

# Fresnelkoeffizient
plt.figure(figsize=(8,4))
plt.plot(f_shift[f_shift >= 0]/1e6, np.abs(R_freq[f_shift >= 0]))
plt.title("|R(f)| Fresnel Reflexionskoeffizient")
plt.xlabel("Frequenz (MHz)")
plt.ylabel("|R(f)|")
plt.grid(True)
plt.show()

# Kombiniertes Zeitsignal
plt.figure(figsize=(8,4))
plt.plot(t*1e6, s_comb)
plt.title("Direkter + reflektierter Puls (Zeitbereich)")
plt.xlabel("Zeit (µs)")
plt.ylabel("Amplitude")
plt.grid(True)
plt.show()

# kombiniertes Spektrum
S_comb_mag = 20 * np.log10(np.abs(np.fft.fftshift(S_comb)) + 1e-20)

plt.figure(figsize=(8,4))
plt.plot(f_shift/1e6, S_comb_mag)
plt.title("Spektrum: Direkter + reflektierter Puls")
plt.xlabel("Frequenz (MHz)")
plt.ylabel("Magnitude (dB)")
plt.grid(True)
plt.xlim(0, fs/2/1e6)
plt.show()

print("\nSimulation erfolgreich abgeschlossen.")
