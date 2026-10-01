# enob.py  -  SINAD and ENOB from your own sampled sine.
# Usage:  python enob.py sine-10k-100n.txt [--fs 1000] [--cycles 32]
#
# ENOB is not a property of a chip. It is the answer to a measurement, and the
# measurement is defined on a sine wave: put in a tone you know, take out
# everything that is not that tone, and ask how many bits of an ideal converter
# would have been that bad. Nothing else you feed a converter has a known enough
# truth for the question to mean anything.
#
# WHOSE ENOB. Sketch F's tone is made by the sigma-delta modulator, smoothed by
# your R and your C, and only then read by the ADC. The modulator's quantisation
# and the filter's contribution are inside the "everything else" this script
# measures, so the figures below belong to that WHOLE CHAIN and not to the
# converter alone. Isolating the converter would need a signal source known to be
# better than the converter, which this rig does not have. Report it as the
# chain's ENOB - and note that the comparison between two R/C configurations, and
# the error budget in cycle 3, are both questions about the chain anyway.
#
#   SINAD = 10*log10( power in the fundamental bin / power in every other bin )
#
#   Only bin 0 (DC) is thrown away. One bin is signal; everything else - leakage,
#   harmonics, drift, hum, quantisation - is noise and distortion. If a neighbour
#   bin were counted as signal, an interferer sitting next to your tone would be
#   invisible, which is the one thing this script must not do.
#   ENOB  = (SINAD - 1.76) / 6.02
#
# Sketch F makes the generator and the sampler share one clock, so a whole number
# of cycles lands in the record. That is called COHERENT sampling and it is why
# this script uses no window: with coherence there is nothing to leak.
import sys
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

FULL_SCALE = 4096.0


def arg(name, default):
    if name in sys.argv:
        return float(sys.argv[sys.argv.index(name) + 1])
    return default


def main():
    if len(sys.argv) < 2:
        raise SystemExit("usage: python enob.py <sine file> [--fs 1000] [--cycles 32]")
    path   = sys.argv[1]
    fs     = arg("--fs", 1000.0)
    cycles = arg("--cycles", 32.0)

    codes = np.array([int(x) for x in open(path) if x.strip().isdigit()], float)
    n = codes.size
    if n < 256:
        raise SystemExit("%s: need at least 256 numeric lines, got %d. Did the run finish?" % (path, n))

    lo, hi = codes.min(), codes.max()
    amp_pp = hi - lo
    print("file                   ", path)
    print("n                      ", n, " (%.3f s at %g Hz)" % (n / fs, fs))
    print("min / max       (codes)", int(lo), "/", int(hi))
    print("peak-to-peak    (codes)", int(amp_pp), " = %.1f %% of full scale" % (100.0 * amp_pp / FULL_SCALE))

    if lo <= 0 or hi >= 4095:
        print()
        print("*** CLIPPED. The record touches a rail, so the flat top is being counted as")
        print("*** distortion and the ENOB below is not a measurement of the converter.")
        print("*** Fix the input range first, then take the number.")

    x = codes - codes.mean()
    if np.allclose(x, 0.0):
        raise SystemExit("every sample is identical. There is no sine here - check the wiring "
                         "and that GPIO2 is actually driving the resistor.")

    # Rectangular window: coherent capture, nothing to leak.
    X = np.abs(np.fft.rfft(x))
    p = X ** 2
    freq = np.fft.rfftfreq(n, 1.0 / fs)

    # DC carries no information about the converter and is removed above by
    # subtracting the mean, so bin 0 is excluded from BOTH signal and noise.
    # Nothing else is excluded. Every other bin is noise or distortion, including
    # the bins either side of the fundamental and the low bins that carry drift -
    # those are exactly what this measurement exists to catch.
    p[0] = 0.0

    k = int(np.argmax(p))
    expect = int(round(cycles))
    print("fundamental at bin     ", k, " = %.4f Hz" % freq[k])
    if k != expect:
        print("  NOTE expected bin %d (%g cycles in the record). Yours is %d." % (expect, cycles, k))
        print("  Either the sample rate argument is wrong or the capture is not coherent.")
        print("  The SINAD below is NOT usable. Fix the capture, do not report this number.")

    # The capture is coherent and unwindowed, so a pure tone lands in exactly one
    # bin. Anything in a neighbouring bin is leakage or interference - not signal.
    p_sig = float(p[k])

    mask = np.ones(p.size, bool)
    mask[k] = False
    mask[0] = False
    p_nd = float(p[mask].sum())

    if p_nd <= 0.0:
        raise SystemExit("noise-plus-distortion power is exactly zero. That is not physical - "
                         "the record is probably a repeated pattern rather than a measurement.")

    sinad = 10.0 * np.log10(p_sig / p_nd)
    enob  = (sinad - 1.76) / 6.02

    # Peak amplitude of the fundamental, in codes, from the same spectrum.
    a_pk = 2.0 * X[k] / n

    print("fundamental amplitude  ", round(float(a_pk), 2), "codes peak")
    print("SINAD of the chain (dB)", round(float(sinad), 2))
    print("ENOB of the chain(bits)", round(float(enob), 2))

    # Worst harmonic, for the report. Anything at 2f, 3f ... that is not the tone.
    worst_h, worst_db = None, None
    for h in range(2, 8):
        b = k * h
        if b >= p.size - 1:
            break
        bb = int(np.argmax(p[b - 1:b + 2])) + b - 1
        d = 10.0 * np.log10((p[bb] + 1e-30) / p_sig)
        if worst_db is None or d > worst_db:
            worst_db, worst_h = float(d), h
    if worst_h:
        print("worst harmonic         ", "%dx fundamental, %.1f dB below it" % (worst_h, -worst_db))

    print()
    print("This is the ENOB of the whole chain: sigma-delta generator -> your R and C")
    print("-> the ADC. The generator's quantisation and the filter are counted as")
    print("error along with the converter's, and this rig cannot separate them.")
    print("ENOB is only comparable between two measurements taken at the SAME input")
    print("amplitude. Yours filled %.1f %% of full scale. Write that down next to the" % (100.0 * amp_pp / FULL_SCALE))
    print("number, or the number cannot be compared with anybody else's.")

    db = 10.0 * np.log10(p + 1e-30)
    db -= db[k]
    plt.figure(figsize=(7, 4))
    plt.plot(freq, db, lw=1)
    plt.xlabel("Hz")
    plt.ylabel("dB below the fundamental")
    plt.title("%s   chain SINAD %.2f dB   chain ENOB %.2f bits" % (path, sinad, enob))
    plt.ylim(-100, 5)
    plt.xlim(0, fs / 2.0)                          # FIXED axes. Two desks have to be comparable.
    plt.tight_layout()
    plt.savefig(path + ".png", dpi=150)
    print("wrote                  ", path + ".png")


if __name__ == "__main__":
    main()
