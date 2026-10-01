# hum.py  -  is the mains in your signal?
# Usage:  python hum.py wire-touch.txt [mV_per_code]
#
# The second argument is optional and it is YOUR number: the mV-per-code scale
# factor off a Sketch E or Sketch F "scale factor" banner line. Without it the
# answer comes out in ADC codes, which is still an answer - and if cycle 1 would
# only rail, come back and re-run this line once Sketch F has given you one.
import sys
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

FS = 250.0                                        # the rate you verified in W03

if len(sys.argv) < 2:
    sys.exit("usage: python hum.py <raw codes file> [mV_per_code]")

codes = np.array([int(x) for x in open(sys.argv[1]) if x.strip().isdigit()], float)
if codes.size < 256:
    sys.exit("%s: need at least 256 numeric lines, got %d" % (sys.argv[1], codes.size))

mv_per_code = float(sys.argv[2]) if len(sys.argv) > 2 else None

x    = codes - codes.mean()                       # the DC level is not the question here
win  = np.hanning(x.size)
X    = np.abs(np.fft.rfft(x * win))
freq = np.fft.rfftfreq(x.size, 1.0 / FS)
amp  = 2.0 * X / win.sum()                        # window-corrected, in ADC codes

band = (freq > 55) & (freq < 65)
k    = int(np.argmax(amp[band]))
f60  = float(freq[band][k])
a60  = float(amp[band][k])

quiet = (freq > 80) & (freq < 120)                # away from mains and its harmonics
floor = float(np.median(amp[quiet]))

print("file                  ", sys.argv[1])
print("n                     ", x.size, " (%.1f s at %.0f Hz)" % (x.size / FS, FS))
print("peak in 55-65 Hz      ", round(f60, 2), "Hz")
print("its amplitude  (codes)", round(a60, 3))
print("broadband 80-120 Hz   ", round(floor, 3), "codes (median)")
print("mains above the floor ", round(20 * np.log10(a60 / (floor + 1e-12)), 1), "dB")
if mv_per_code:
    print("mains amplitude       ", round(a60 * mv_per_code * 1000.0, 1), "uV")
    print("  for scale: an ECG R wave at the skin is about 1000 uV, before any amplifier")
else:
    print("  no mV-per-code given, so the answer is in codes. Pass yours as a second")
    print("  argument to get microvolts:  python hum.py %s 0.xxxx" % sys.argv[1])

db = 20 * np.log10(amp + 1e-12)
db -= db.max()
plt.figure(figsize=(7, 4))
plt.plot(freq, db, lw=1)
plt.axvline(60, color="r", ls="--", lw=1)
plt.xlabel("Hz")
plt.ylabel("dB below the largest component")
plt.title(sys.argv[1])
plt.ylim(-80, 5)
plt.xlim(0, 125)                                  # FIXED axes, so two desks can be compared
plt.tight_layout()
plt.savefig(sys.argv[1] + ".png", dpi=150)
print("wrote                 ", sys.argv[1] + ".png")
