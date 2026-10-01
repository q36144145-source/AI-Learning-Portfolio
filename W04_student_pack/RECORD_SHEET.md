# W04 Record Sheet — Can your board see an ECG?

**Name** _______陳竑勳_______  **Student ID** ______Q36144145______  **Date** 2026-10-01

Every cell below is measured by you, in this room, today, or carried forward from a
measurement you made in W03. **There is no reference answer on any slide in this deck and
none in this folder.** If a cell is empty at 12:00, say so — an empty cell is a result, a
guessed cell is not.

The sections below are in the order HW2 asks for them. Fill this in and most of HW2 is
already written.

---

## 1 · Your converter on its own (cycle 1)

Four runs of Sketch E. Change one line, reflash, run.

| Run | Input on A0 | `ATTEN` | `BIAS` | Mean (codes) | Std dev (codes) | Peak-to-peak (codes) | Noise-free bits |
|---|---|---|---|---|---|---|---|
| 1 | jumper to **GND** | `ADC_11db` | 0 |0.152 |0.8113 |12 |8.42 |
| 2 | **nothing on A0** | `ADC_11db` | 1 |1755.807 |1.0413 |15 |8.09 |
| 3 | **nothing on A0** | `ADC_0db` | 1 |4095 |0.0 |1 |12.0 |
| 4 | 20 cm jumper, held | `ADC_11db` | 0 |1067.149 |1202.9193 |3754 |0.13 |

**The scale-factor block from run 2's banner**, copied verbatim — including a
`NOT COMPUTABLE` refusal, if that is what it printed:

```
# mV and raw code  :1499.1 mV, code 1751.8 (two separate reads)
# scale factor     :0.8558 mV per code, at that one point
# the same x 4095  :3504 mV - extrapolation, not a measured full scale
```

**Your scale factor.** This is a ratio: millivolts over codes, from **two separate reads**,
where the millivolt side is the SDK's own calibration curve applied to its own raw read. It
converts codes to millivolts, which is all this week needs. **It is not a calibration** — one
point cannot show the curve is straight elsewhere, and nothing here was compared with an
external reference. Say where yours came from.

| | Value | Unit |
|---|---|---|
| **Scale factor** |0.8558 | mV per code |
| Extrapolated full scale = scale factor × 4095 — *an extrapolation, not a measured full scale* |3504 | mV |
| DC offset — the mean of run 1, with the pin at 0 V |0.152 | codes |
| The same offset in millivolts |0.1301 | mV |

**The refusals — what happened, in one sentence each.** Expect **runs 1 and 3** to refuse:
run 1 sits on the bottom rail, and run 3 puts the same voltage run 2 measured past the top of
a smaller range. Do not write down a number a tool refused to give you; write down what it
refused and why.

> Run 1: Printed `NOT COMPUTABLE - the input is sitting on a rail.` because A0 is shorted directly to GND (0 V), sitting on the bottom rail (`WARNING: railed at the bottom.`)
>
> Run 3: Printed `NOT COMPUTABLE - the input is sitting on a rail.` because the ~1499.1 mV mid-rail bias voltage exceeds the smaller `ADC_0db` range, saturating all 2000 samples at the top rail of code 4095 (`WARNING: railed at the top.`).
>
> Run 2 computed — if it did **not**, say what was on A0: Computed `0.8558 mV per code` normally (nothing external on A0; held at mid-scale by internal pull-up and pull-down resistors).

**The DC figure is not ENOB.** `adcstats.py` prints "bits from DC noise". Write it here for
run 1 — and then say, in your own words, one thing it cannot see:

| Run 1 bits from DC noise |10.51 |
|---|---|

> One thing a static DC noise test cannot see | Harmonic distortion (INL / non-linearity) and sampling clock jitter, which only appear when the input signal varies dynamically over time.

## 2 · What the room puts into a floating wire (cycle 1)

From run 4 and `hum.py`. Give `hum.py` **your own** scale factor — the one run 2 printed — as
the second argument. Do not borrow anyone else's number; every chip's calibration is its own.

| | Value | Unit |
|---|---|---|
| Peak frequency found in the 55–65 Hz band | 60.0 | Hz |
| Its amplitude | 1471.852 | codes |
| The same amplitude | 1259610.8 | µV |
| Broadband floor, 80–120 Hz | 6.453 | codes |
| Mains above the floor | 47.2 | dB |

**An R wave at the skin is about 1 mV.** Using your own scale factor, how many codes is
1 mV, and how does that compare with the 60 Hz amplitude you just measured?

| 1 mV in codes | 1.17 |
|---|---|
| 60 Hz amplitude in codes | 1471.852 |
| Ratio, hum ÷ signal | 1259.61 |

> One sentence on what that ratio means for a single-ended ECG input:
>
> Because the 60 Hz mains hum coupled into the body (1471.852 codes, or 1.26 V peak) is 1259.61 times larger than an unamplified 1 mV R wave (1.17 codes), a single-ended ECG input without a differential instrumentation amplifier (high CMRR) and right-leg drive will completely bury the cardiac signal in power-line interference.

## 3 · SINAD and ENOB — of the chain, from your own sine (cycle 2)

Two runs of Sketch F. **The sketch does not change. The breadboard does.**

**Whose ENOB this is.** The tone was made by the sigma-delta modulator, smoothed by your R and
C, and only then read by the ADC. Everything `enob.py` counts as noise and distortion therefore
includes the **generator's own quantisation** and whatever your **filter** contributed.
So these figures belong to the whole signal chain, not to the converter. Separating out the
converter would need a source known to be better than the converter, and this rig has none.
**Write "chain ENOB" wherever you report it**, here and in HW2.

| Config | R | C | Peak-to-peak (codes) | % of full scale | Chain SINAD (dB) | **Chain ENOB (bits)** |
|---|---|---|---|---|---|---|
| A | 10 kΩ | 100 nF | 2743 | 67.0 % | 27.62 | 4.30 bits (at 67.0 % FS) |
| B | 100 kΩ | 10 nF | 2726 | 66.6 % | 34.00 | 5.36 bits (at 66.6 % FS) |

| | A | B |
|---|---|---|
| Fundamental amplitude (codes peak) | 1324.72 | 1322.13 |
| Worst harmonic, dB below fundamental | 2x fundamental, 40.6 dB below | 2x fundamental, 36.4 dB below |
| Worst sampling-interval error the sketch reported (µs) | 3 | 2 |

**Both configurations have the same R × C.** So state what did and did not change:

> What changed between A and B, in one sentence:
>
> Although the RC time constant (1 ms, corner ~159 Hz) and fundamental amplitude (~66.6–67.0 % of full scale) remained virtually unchanged, increasing the source resistance tenfold to 100 kΩ and shrinking the storage capacitor tenfold to 10 nF lowered the broadband noise floor (raising chain SINAD from 27.62 dB to 34.00 dB and chain ENOB from 4.30 bits to 5.36 bits), while slightly worsening the 2nd harmonic distortion from 40.6 dB to 36.4 dB below the fundamental due to higher source impedance interacting with the SAR ADC's switched-capacitor input.
>
> The difference in ENOB, in bits: 1.06 bits (5.36 - 4.30)
>
> Is that difference larger than the difference between two repeats of the *same*
> configuration? If you did not repeat one, say so:
>
> I did not repeat either configuration, so while a 1.06-bit (6.38 dB SINAD) shift is substantial, I cannot formally bound the within-configuration repeatability spread from a single run per setup.

**Which of the two would you quote as your signal chain's ENOB, and why?** This is a
judgement, not a maximum.

> I would quote Config A's chain ENOB of 4.30 bits (at 67.0 % of full scale) because a 10 kΩ source impedance with a 100 nF charge-reservoir capacitor properly buffers the SAR ADC's internal sampling capacitor kickback (yielding 4.2 dB cleaner 2nd-harmonic linearity at 40.6 dB below fundamental) and gives a conservative, realistic lower bound for the error budget. 

## 4 · Carried forward from W03 (required — HW1 data)

From your `w03/RECORD_SHEET.md`. If you cannot find it, write **"not recovered"** and say
what you tried. Do not leave it blank.

| | Value | Unit |
|---|---|---|
| Std dev of sampling interval, timer version (Sketch B) | 0.0023 | ms |
| Std dev of sampling interval, `delay(4)` version (Sketch A) | 1.0023 | ms |
| Which one you would use, from W03 | Timer ISR version (Sketch B, σt = 0.0023 ms), because its sampling jitter is 435.78 times smaller than `delay(4)` and it is the only version usable for HRV/ECG timing. | |

## 5 · The error budget — can this board see an ECG?

Everything here is arithmetic on the rows above. Show the arithmetic.

**Step 1 — what one effective code is worth.**

> effective LSB = (extrapolated full scale in mV) ÷ 2^(chain ENOB)
>
> = 3504 mV ÷ 2^4.30 = 177883.3 µV  (Ideal 12-bit LSB = 3504 mV ÷ 2^12 = 855.47 µV; ratio = 207.94×)
>
> Both inputs carry a caveat: the numerator is extrapolated from one point, and the
> exponent is the chain's, not the converter's. Carry both into section 7.

**Step 2 — how much of that a 1 mV signal occupies.**

> 1 mV ÷ (effective LSB) = 1000 µV ÷ 177883.3 µV = 0.00562 effective codes

**Step 3 — what your own jitter costs.** For a sine of amplitude *A* at frequency *f*
sampled with timing standard deviation σ*t*, the voltage error it produces is

> ΔV(rms) = 2π · f · A · σ*t*

Choose *f* yourself and **say why you chose it** — the ECG band does not have one frequency,
and which one you pick is a design decision you are answerable for.

| | Value | Unit |
|---|---|---|
| f you chose | 40 | Hz |
| Why you chose it | Standard upper cutoff of clinical monitoring ECG bandwidth (0.5–40 Hz), covering the dominant energy of the QRS complex. | |
| A (use 1 mV) | 1.000 | mV |
| σ*t* from section 4 | 0.0000023 | s |
| ΔV(rms) | 0.5781 | µV |
| ΔV in effective codes | 0.00000325 | codes |

**Step 4 — the answer.** One sentence, with your own numbers inside it. Not "it is not
good enough" — that is a mood, not a result.

> Although my W03 hardware timer jitter (σt = 0.0000023 s) adds only 0.5781 µV rms of timing noise (0.00000325 effective codes at 40 Hz, leaving 0.00562 codes after subtraction), the 3504 mV extrapolated full scale and 4.30-bit chain ENOB yield an effective LSB of 177,883.3 µV, meaning an unamplified 1 mV ECG R wave occupies just 0.00562 effective codes and is buried under 1,259,610.8 µV of 60 Hz mains hum without an external analog front end.

## 6 · What you would have to buy (cycle 3, Principle 03)

For each row: the figure you got from the AI, then **where in the manufacturer's own data
sheet you found it** — page, table or section number. If you could not find it, write
**"not located"**. That is the honest entry and it is marked as one.

| Parameter | What the AI said | Page / table / section in the data sheet | Agrees? |
|---|---|---|---|
| Gain | 100 V/V fixed (instrumentation amplifier), with external op-amp A1 for additional gain (typically 1100 V/V total) | Page 1 (Features) & Page 3, Table 1 (Specifications — Instrumentation Amplifier, Gain = 100 V/V) | Yes |
| CMRR | 80 dB min (86 dB typ), DC to 60 Hz | Page 3, Table 1 (Specifications — Common-Mode Rejection Ratio, 80 dB min at DC to 60 Hz) | Yes |
| Bandwidth (the filter corners) | Set by external passive RC components (e.g., 0.5–40 Hz or 0.05–150 Hz); internal op-amp GBP is 100 kHz | Page 4, Table 1 (Op-Amp Gain Bandwidth Product = 100 kHz) & Pages 18–21 (Applications Information — Filter Design) | Yes |
| Right-leg drive — what it is *for* | Inverts the common-mode signal to drive the body via a third electrode and cancel 50/60 Hz power-line interference | Page 15 (Theory of Operation — Right-Leg Drive Amplifier) | Yes |

**Anything the AI stated that you could not locate anywhere in the document:**

> Specific fixed filter corner frequencies (such as 0.5 Hz to 40 Hz as an intrinsic chip parameter) are not fixed specifications in Table 1 because the AD8232 has no internal filter capacitors; all high-pass and low-pass cutoff frequencies are determined entirely by external resistors and capacitors chosen in the application circuit (Pages 18–21).

**One sentence on what a dedicated front end sells you that your board does not have** —
in terms of the numbers in sections 1, 3 and 5.

> A dedicated front end like the AD8232 provides 80 dB of common-mode rejection with right-leg drive to suppress the 1,259,610.8 µV (1471.852 codes) of 60 Hz mains hum, along with 100× instrumentation amplifier gain (expandable to 1100×) to lift a 1 mV R wave from an unresolvable 0.00562 effective codes (against my 3504 mV full scale and 4.30-bit chain ENOB, where 1 effective LSB = 177,883.3 µV) into hundreds of millivolts that span multiple effective codes.

## 7 · Method note (one sentence, required)

> The sine was generated by the same hardware timer that sampled it, so the capture is
> coherent and the FFT used no window. The SINAD and ENOB are the whole chain's — the
> sigma-delta generator, the RC I built, and the converter — because nothing in the rig can
> separate them. The millivolts-per-code is a single-point ratio taken from the SDK's own
> calibration, not an independent calibration, and the full scale quoted above is
> extrapolated from that one point.

Anything else a reader would need in order to repeat what you did:

> Cycle 1 captured 2000 samples at 250 Hz on GPIO1 (A0) using Sketch E, while Cycle 2 drove a 15.625 Hz sine from GPIO2's sigma-delta modulator (carrier ~1220 Hz, edge rate ~312.3 kHz) through a 1-pole breadboard RC filter (Config A: 10 kΩ + 100 nF; Config B: 100 kΩ + 10 nF) into GPIO1 with `ADC_11db` attenuation and `BIAS = 1`, capturing 2048 samples at 1000 Hz (32 exact cycles, bin 32).

## 8 · Bonus, not required

*Raise your chain's ENOB.* Knobs with no new parts: the attenuation setting, the sample rate,
oversampling and averaging, the R and C values, the sigma-delta carrier frequency. Some of
these improve the generator rather than the converter — say which part of the chain you moved.

| | Chain ENOB (bits) | % of full scale | What it cost |
|---|---|---|---|
| Before | | | — |
| After | | | |

**What you changed, and the price you paid for it.** Oversampling buys bits with bandwidth.
Averaging buys them with latency. A bigger number with no named cost scores nothing.

> _______________________________________________________________

---

## Before you leave

- [ ] `gnd-12db.txt`, `mid-12db.txt`, `mid-0db.txt`, `wire-touch.txt` in `w04/`
- [ ] `wire-touch.txt.png` in `w04/`
- [ ] `sine-10k-100n.txt` and `sine-100k-10n.txt` in `w04/`, plus both `.png` spectra
- [ ] **`sine.txt`** — a copy of the run you would defend. W06 filters this one.
- [ ] Both scripts committed, so your numbers can be reproduced
- [ ] Every `NOT COMPUTABLE` refusal written down, and no cell filled with a borrowed number
- [ ] Two entries in `AI_USAGE.md`
- [ ] This sheet, filled in
- [ ] Google Form submitted **before you leave the room**

Files in the repo the same day. You cannot re-create any of this once the board is
reflashed and the breadboard is pulled apart.
