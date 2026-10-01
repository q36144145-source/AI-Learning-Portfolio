# W04 Student Pack — ADC and the Analog Front End

**Embedded Systems (BME 3301) · Week 04 · Thursday 1 Oct, 09:10–12:00 · 醫技醫工館 5776**

Everything you run today is in this folder. Nothing has to be typed off the slides — the
files here are character-for-character identical to what is projected.

**Unzip this into its own folder and leave it there.** Your PlatformIO project is a
*separate* folder: you copy `platformio.ini` and one sketch out of here and into it. Each
week of this course has its own pack with files of the same names, so a pack unzipped on top
of another one overwrites the record sheet you have been filling in.

**`PREFLIGHT.md` is required before class.** About fifteen minutes. Two of its four items
cost nothing and the other two will cost you the morning if you skip them.

---

## Read this first — what changed, and why you are being told

The ECG front-end modules for this course were never ordered and are not here. Week 4 as
originally written put an AD8232 breakout on every desk, three electrodes on a classmate,
and asked you to look at the trace.

We are not doing a watered-down version of that. **Today the object of study is the
converter itself**, which you have forty-five of in this room, and which is genuinely the
harder half of the subject. You will measure what your own signal chain is actually worth,
in bits, using a signal whose truth you know because you generated it — the converter with
the generator and the filter you built in front of it, because that is the thing this rig can
honestly measure. At the end of the
morning you will use that number to answer a real question — *could this board, on its own,
acquire an ECG?* — and the answer will be yours, computed from your own measurements.

Naming a constraint out loud and then being accountable for the design decision it forces
is the whole point of this course. This week it is our constraint, not yours.

---

## What is in here

```
LAB_GUIDE.md              every run, step by step, at your own pace. Open it and work down it.
platformio.ini            W03's file, unchanged. Same board line, same monitor_filters.
sketches/
  sketch_e_adc.cpp        the converter on its own                        (cycle 1)
  sketch_f_sine.cpp       a known sine, generated and sampled by one clock (cycle 2)
  sketch_g_steps.cpp      BONUS, not run in class - the transfer curve
scripts/
  adcstats.py             raw codes -> mean, sigma, peak-to-peak, noise-free bits
  hum.py                  a 250 Hz trace -> the 60 Hz amplitude + a fixed-axis spectrum
  enob.py                 your sampled sine -> the CHAIN's SINAD and ENOB
  linfit.py               BONUS - Sketch G's steps -> the worst bend in the curve
RECORD_SHEET.md           fill in as you go. It is the deliverable.
AI_USAGE_template.md      the two entries you owe in AI_USAGE.md
PREFLIGHT.md              do this before class
```

`adcstats.py` uses **only the standard library**. If `numpy` will not install, cycle 1
still works completely.

### Two words this pack is careful about

**Scale factor, not calibration.** Sketch E and Sketch F both print a `mV per code` figure.
It is `analogReadMilliVolts()` divided by `analogRead()` — **two separate reads**, where the
millivolt side is already the SDK's own calibration curve applied to its own raw read. So the
ratio is that curve's slope at one operating point. It converts codes to millivolts, which is
all this week needs. It is **not** a calibration: one point cannot demonstrate linearity, and
no independent voltage reference exists on any desk in this room. Multiplying it by 4095 gives
an **extrapolated** full scale, not a measured one, and the sketch labels it that way.

**Chain ENOB, not converter ENOB.** Sketch F's tone is made by the sigma-delta modulator,
smoothed by the R and C you plug in, and only then read by the ADC. Everything `enob.py` counts
as noise and distortion includes the generator's own quantisation and the filter's
contribution. Isolating the converter would need a source known to be better than the
converter, and this rig has none. Report it as the chain's — which is the more useful question
anyway, and the one cycle 3's error budget asks.

### One naming oddity, so it does not trip you up

The sketches use the constant `ADC_11db`, but the filenames say `12db`. Both are correct.
The Arduino core still exposes the old `ADC_11db` name, and Espressif's own header marks the
corresponding IDF constant **deprecated**, stating that it behaves identically to the 12 dB
one. The files are named for what the setting actually is, and the code is written with the
name the compiler accepts. You will look at that header in cycle 3.

## What you build on the breadboard

Two components and three jumper wires. That is the whole circuit for the week.

```
GPIO2 ---[ R ]---+--- GPIO1 (A0)
                 |
               [ C ]
                 |
                GND
```

You build it **twice**, with two pairs that have the *same* corner frequency and *different*
source resistance:

| Config | R | C | R × C corner | What is being changed |
|---|---|---|---|---|
| A | 10 kΩ | 100 nF | ≈ 159 Hz | — |
| B | 100 kΩ | 10 nF | ≈ 159 Hz | source resistance, ten times higher |

Same filter, same signal, same sketch. The only thing that differs is how hard it is for
the converter's input to pull charge out of the node it is measuring. Whatever difference
you measure between A and B is what "input impedance" means in practice.

**GPIO2 and GPIO1 are next to each other on the right-hand header**, two rows below `RX`.
Count them on the silkscreen before you plug anything in.

## What you produce today

All of it goes in your repo under `w04/`:

| File | From | Cycle |
|---|---|---|
| `gnd-12db.txt` | Sketch E, pin jumpered to GND | 1 |
| `mid-12db.txt` | Sketch E, **nothing on A0**, `BIAS 1` | 1 |
| `mid-0db.txt` | Sketch E, same bare pin, `ATTEN` changed to `ADC_0db` | 1 |
| `wire-touch.txt` | Sketch E, 20 cm jumper held between finger and thumb | 1 |
| `wire-touch.txt.png` | `hum.py` — fixed axes, so two desks are comparable | 1 |
| `sine-10k-100n.txt` | Sketch F, config A — **also the banner your scale factor comes from** | 2 |
| `sine-100k-10n.txt` | Sketch F, config B | 2 |
| both `.png` spectra | `enob.py` | 2 |
| `sine.txt` | **a copy of whichever sine run you would defend.** W06 uses this | 2 |
| `RECORD_SHEET.md` | filled in | all |

Plus two entries in `AI_USAGE.md`.

## How your data gets off the board

Same as W03. `monitor_filters = log2file` writes everything the monitor receives into a
`platformio-device-monitor-*.log` in your project folder. After each run take the **newest**
log, rename it, and move it into `w04/`. Leave the `#` banner lines in — every script here
skips any line that is not entirely digits, and the banners carry the settings the file
itself cannot.

**Sketch F prints which settings it used but it cannot know which resistor you plugged in.**
Write `10k / 100n` or `100k / 10n` into the filename. Two files with the same name and
different resistors is the one mistake in this week that cannot be recovered afterwards.

## The signal contract — unchanged since W03, and W06 depends on it

```
A signal file is plain text.
One integer per line. Nothing else on the line.
Lines that are not entirely digits are banner text and are ignored.
The integer is a raw ADC count, 0 to 4095.
The SAMPLE RATE IS NOT IN THE FILE. It is stated separately.
```

Cycle 1's files are at **250 Hz**. Cycle 2's are at **1000 Hz**. Nothing in either file says
so, and every tool you point at them will run happily with the wrong rate and give you
frequencies that are wrong by exactly that factor. That is not a bug in the tools. It is
what an interface that carries numbers and not meaning costs you, and the cost is paid by
whoever forgets — which in three weeks' time is you.

## Units

Counts in the files. **Millivolts** wherever you report a voltage, and **microvolts** when
the number would otherwise be a decimal with three leading zeros. **Bits** to two decimal
places for ENOB. **Decibels** to one. Milliseconds to four for anything you carry forward
from W03.

A number also needs the thing it is **about**. "ENOB _._ bits" is incomplete; "**chain** ENOB
_._ bits at __ % of full scale" is a measurement. Same for the scale factor: say which run it
came from, and call the × 4095 figure an extrapolation.

A number without a unit is not a measurement.

## No reference answers

There is no ADC noise figure, no ENOB and no SINAD anywhere in the deck or in this folder.
Every one of them is measured in this room today, on your board, by you. The only numbers
that appear on a slide are Flash and RAM byte counts, because the linker prints the same
figure on every desk.
