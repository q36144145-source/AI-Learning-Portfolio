# linfit.py  -  BONUS. Is the curve between the two ends straight?
# Usage:  python linfit.py steps.txt
#
# Input is Sketch G's two columns: duty,code  ("#" lines ignored).
#
# A standard deviation taken at one input tells you nothing about the shape of
# the transfer curve. Averaging does not fix a bend. This script fits ONE straight
# line through the mean code at every step and reports the worst distance from it.
# That distance is the part of the error no amount of repetition will remove.
import sys
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

FULL_SCALE = 4096.0

if len(sys.argv) < 2:
    sys.exit("usage: python linfit.py <steps file>")

groups = {}
for line in open(sys.argv[1]):
    s = line.strip()
    if not s or s.startswith("#"):
        continue
    parts = s.split(",")
    if len(parts) != 2:
        continue
    try:
        d, c = int(parts[0]), int(parts[1])
    except ValueError:
        continue
    groups.setdefault(d, []).append(c)

if len(groups) < 4:
    sys.exit("%s: found %d steps. Sketch G writes 17. Wrong file, or the run did not finish."
             % (sys.argv[1], len(groups)))

duty = np.array(sorted(groups), float)
code = np.array([np.mean(groups[int(d)]) for d in duty], float)
sd   = np.array([np.std(groups[int(d)], ddof=1) for d in duty], float)

# A step is railed when its MEAN is on a rail, or when most of its READINGS are:
# noise can pull the mean of a step sitting on 4095 a few codes down, and that
# step would otherwise stay in the fit and look like the worst bend on the curve.
on_rail = np.array([np.mean([(c <= 0) or (c >= 4095) for c in groups[int(d)]]) for d in duty])
railed = (code <= 1) | (code >= 4094) | (on_rail > 0.5)
if railed.any():
    print("dropping %d railed step(s) from the fit: duty %s"
          % (railed.sum(), ", ".join(str(int(d)) for d in duty[railed])))
    print("  a railed step is not a point on the curve, it is the end of it")
touching = ~railed & (on_rail > 0)
if touching.any():
    print("NOTE %d step(s) kept, but some of their readings sit on a rail: duty %s"
          % (touching.sum(), ", ".join(str(int(d)) for d in duty[touching])))
    print("  their means are pulled toward the middle, so treat those points with care")
use = ~railed
if use.sum() < 4:
    sys.exit("fewer than 4 unrailed steps. Lower the attenuation setting or check the wiring.")

a, b = np.polyfit(duty[use], code[use], 1)
resid = code[use] - (a * duty[use] + b)
worst = float(np.max(np.abs(resid)))

print("file                   ", sys.argv[1])
print("steps used             ", int(use.sum()), "of", len(duty))
print("best straight line      code = %.4f * duty %s %.2f" % (a, "-" if b < 0 else "+", abs(b)))
print("worst departure  (codes)", round(worst, 2))
print("  as a fraction of FS   ", round(100.0 * worst / FULL_SCALE, 3), "%")
print("  bits left if this bend", round(float(np.log2(FULL_SCALE / max(2.0 * worst, 1.0))), 2),
      "noise-free bits - i.e. the bend alone caps you here")

diffs = np.diff(code[use])
if np.any(diffs <= 0):
    print("NON-MONOTONIC: the mean code went DOWN when the duty went UP, at step(s)",
          ", ".join(str(int(d)) for d in duty[use][:-1][diffs <= 0]))
    print("  before blaming the converter, check SETTLE_MS in Sketch G against your own R*C")
else:
    print("monotonic              yes")

print("within-step stdev       %.2f .. %.2f codes" % (sd[use].min(), sd[use].max()))

fig, ax = plt.subplots(2, 1, figsize=(7, 6), sharex=True)
ax[0].plot(duty, code, "o-", lw=1, ms=4)
ax[0].plot(duty[use], a * duty[use] + b, "r--", lw=1)
ax[0].set_ylabel("mean code")
ax[0].set_ylim(0, 4096)
ax[1].plot(duty[use], resid, "o-", lw=1, ms=4)
ax[1].axhline(0, color="r", ls="--", lw=1)
ax[1].set_xlabel("sigma-delta duty")
ax[1].set_ylabel("departure from the line (codes)")
ax[1].set_xlim(0, 256)
fig.suptitle(sys.argv[1])
fig.tight_layout()
fig.savefig(sys.argv[1] + ".png", dpi=150)
print("wrote                  ", sys.argv[1] + ".png")
