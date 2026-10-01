# adcstats.py  -  what your converter does when the input is not moving.
# Usage:  python adcstats.py gnd-12db.txt
#
# Standard library only. If numpy would not install last week, this still runs.
import sys, math, statistics as st

FS_CODES = 4096.0                                  # 12 bits, by definition

if len(sys.argv) < 2:
    sys.exit("usage: python adcstats.py <raw codes file>")

codes = [int(x) for x in open(sys.argv[1]) if x.strip().isdigit()]
if len(codes) < 2:
    sys.exit(sys.argv[1] + ": fewer than 2 numeric lines. Wrong file, or the run did not finish.")

mean = st.mean(codes)
sd   = st.stdev(codes)                             # SAMPLE stdev - say so in your report
lo, hi = min(codes), max(codes)
pp   = hi - lo + 1                                 # peak-to-peak in codes, inclusive

print("file                  ", sys.argv[1])
print("n                     ", len(codes))
print("mean          (codes) ", round(mean, 3))
print("stdev         (codes) ", round(sd, 4))
print("min / max     (codes) ", lo, "/", hi)
print("peak-to-peak  (codes) ", pp)

# Two different honest answers to "how many bits is this really?", and they
# disagree on purpose. Quote the definition whenever you quote the number.
#   noise-free bits: how many bits never move, sample to sample.
print("noise-free bits       ", round(math.log2(FS_CODES / pp), 2))
#   the DC figure: full scale over the rms noise, scaled to an ideal quantiser.
#   This is NOT the ENOB you will measure in cycle 2. It cannot see distortion,
#   because a flat input has no shape for the converter to distort.
if sd > 0:
    print("bits from DC noise    ", round(math.log2(FS_CODES / (sd * math.sqrt(12))), 2))
else:
    print("bits from DC noise     not defined - stdev is exactly 0, every sample identical")

if pp == 1:
    print("NOTE: every sample identical. A perfectly still reading is not automatically")
    print("      a good one. Check you are not railed before you celebrate 12 bits.")
if mean >= 4094:
    print("WARNING: railed at the top. This attenuation setting cannot see this voltage.")
if mean <= 1:
    print("WARNING: railed at the bottom.")
