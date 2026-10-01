# W04 Lab Guide — run by run, at your own pace

**Embedded Systems (BME 3301) · Week 04 · Thursday 1 Oct, 09:10–12:00 · 醫技醫工館 5776**

This is the document you drive yourself. The slides move at one speed for forty-five people;
this does not. Work down it in order, at whatever pace your board and your breadboard allow.

Every run has the same three parts:

| | |
|---|---|
| **Do this** | Numbered steps. One action each. Do them in order. |
| **You should see** | The *shape* of the output — the banner, the column, the units, the line count. **Never a value.** Every number today is yours. |
| **If it does not look like that** | The two or three ways this step actually goes wrong, and what to do about each. Read this column **before** you raise your hand — it is where most of the answers live. |

Each run ends with **→ Record sheet**, telling you exactly which cells you can now fill.
Fill them as you go. Not at 11:50.

**Ask a TA when the "If it does not look like that" column says to, or when you have been
stuck on one step for more than about three minutes.** Everything else, keep going.

---

## Before you start — how the screenshots work

Six reference frames belong in this guide. **None have been captured yet** — the TA dry run
produces them. Where one goes, you will find a block like this:

> **📷 SCREENSHOT — NOT CAPTURED YET**
> `example-name.png` — one line saying what the frame shows.

When the frames land in `slides/assets/w04/`, each block becomes
`![...](../../slides/assets/w04/example-name.png)` and nothing around it changes. The six
filenames used here are the same six the deck uses, so a frame only has to be captured once.

---

## Run 0 · Set the project up

**Time: about 8 minutes. Everyone does this once.**

### Do this

1. Unzip `W04_student_pack.zip` somewhere you can find it.
2. Make a new PlatformIO project. Board: **`esp32-s3-devkitc-1`**.
3. **Copy the pack's `platformio.ini` over the generated one.** Overwrite it. It is W03's file
   unchanged, and it carries the two lines that matter: `board = esp32-s3-devkitc-1` and
   `monitor_filters = log2file`.
4. Copy the pack's whole **`scripts/`** folder into the project root, next to `platformio.ini`.
5. Create an empty folder **`w04/`** in the project root. Every file you produce today goes in it.
6. Plug the board in using the **COM** USB-C socket — the one wired through the CH343 bridge.
   The board has two sockets and only this one gives you a serial port with these settings.
   No driver is needed.
7. Confirm the toolchain is awake:
   ```bash
   python -m platformio run
   ```

Your project root should now contain, at minimum:

```
platformio.ini        the pack's, not the generated one
src/                  where each sketch goes, as main.cpp
scripts/              adcstats.py  hum.py  enob.py  linfit.py
w04/                  empty, for now
```

Every command in this guide is run **from the project root**, so `python scripts/adcstats.py
w04/gnd-12db.txt` works exactly as it is written on the slides.

### The three commands, and one rule

```bash
python -m platformio run                      # build          (or Build in the sidebar)
python -m platformio run --target upload      # build + upload (or Upload)
python -m platformio device monitor           # open the serial monitor
```

**The rule: open the monitor first, then press RST.** Every sketch this week does all its work
in `setup()` and then stops, and it dumps two thousand lines in a couple of seconds. Anything
printed before the monitor has attached is gone, and there is no way to get it back except by
running it again. If you prefer one command, `python -m platformio run --target upload --target
monitor` uploads and attaches in one step — press RST once it is up.

### You should see

A build that ends in `[SUCCESS]`, and a `w04/` folder that exists and is empty.

### If it does not look like that

- **No serial port on either socket.** You are on the wrong USB-C socket, or the cable is a
  charge-only cable. Try the other socket first, then another cable. If neither works, open
  `platformio.ini`, uncomment `build_flags = -DARDUINO_USB_CDC_ON_BOOT=1`, rebuild and
  re-upload — `Serial` then becomes the chip's own native USB, which needs no driver anywhere.
- **`python -m platformio run` is not found.** This is the W03 toolchain and nothing about it
  changed. Get a TA now, at 09:15, not at 11:45.
- **You skipped step 3.** You will find out in Run 1, when no `.log` file appears and there is
  nothing to rename. Do step 3.

**→ Record sheet:** nothing yet.

---

# Cycle 1 — the converter on its own

**Four runs of Sketch E. Budget: 24 minutes of hands-on for the whole cycle.**

One run = one condition. You change **one** line, reflash, collect. Nothing else in
`sketch_e_adc.cpp` ever changes.

| Run | A0 (GPIO1) is connected to | `ATTEN` | `BIAS` | Save the log as |
|---|---|---|---|---|
| 1 | a short jumper to **GND** | `ADC_11db` | `0` | `gnd-12db.txt` |
| 2 | **nothing at all** | `ADC_11db` | `1` | `mid-12db.txt` |
| 3 | **nothing at all**, exactly as run 2 | `ADC_0db` | `1` | `mid-0db.txt` |
| 4 | a 20 cm jumper, far end held between finger and thumb | `ADC_11db` | `0` | `wire-touch.txt` |

Runs 2 and 3 have **nothing wired to A0**. `BIAS 1` switches on the pin's own pull-up *and*
pull-down resistors, which are inside the chip; together they are a divider that holds A0
part-way up the supply, at a voltage that is on neither rail. You get a real reading with no
components and no wire.

**Before you plug anything in:** GPIO1 is on the right-hand header, two rows below `RX`. Count
the silkscreen. Putting a jumper into a pin that is driving is how a board dies.

### Two things to know before you start, so you are not stuck at 09:40

**1. Runs 1 and 3 will refuse to give you a `mV per code`, and that is the point.**
Run 1 puts the pin on the bottom rail and run 3 puts it past the top of a range that is too
small for it. Sketch E will not divide on a rail. Copy the refusal into the record sheet
exactly as it printed; a refusal is the entry for those rows, not a missing entry.

**Run 2 is the one that gives you the number.** Nothing is wired to A0, the chip's own two
resistors hold it near the middle, and that point is inside the range the attenuation setting
is specified over. Everything else this morning is denominated in run 2's `mV per code`.

**2. `BIAS` and `ATTEN` are both `#define`s, so changing either needs a rebuild and re-upload
— not a reset.** Runs 2 and 3 each change one of them. This is the single most common way to
lose five minutes today: you press RST, the old firmware runs again, and the numbers do not
move.

- **Do not borrow a number from anyone, and do not reconstruct one.** Every chip has its own
  calibration burned in at the factory, so your neighbour's `mV per code` is not yours. An
  empty cell with a reason beside it is a result; a guessed cell is the one thing this week
  marks as wrong.

---

## Run 1 · A0 jumpered to GND

### Do this

1. Copy `sketches/sketch_e_adc.cpp` into `src/main.cpp`.
2. Put a short jumper from **A0 (GPIO1)** to **GND**. Count the pins on the silkscreen.
3. Build:
   ```bash
   python -m platformio run
   ```
   (or press **Build** in the PlatformIO sidebar — same thing)
4. Read the two resource lines. Check the denominators.
5. Upload:
   ```bash
   python -m platformio run --target upload
   ```
6. Open the monitor, **then press RST** (see the rule in Run 0):
   ```bash
   python -m platformio device monitor
   ```
7. Wait for `# done`.
8. Stop the monitor (`Ctrl+C`, or close the terminal).
9. In the project folder, find the **newest** `platformio-device-monitor-*.log`. Rename it
   `gnd-12db.txt` and move it into `w04/`.
10. ```bash
    python scripts/adcstats.py w04/gnd-12db.txt
    ```

### You should see

The build ends with two resource lines and a success banner:

```
RAM:   [=      ]   9.4% (used 30848 bytes from 327680 bytes)
Flash: [==     ]   8.5% (used 283057 bytes from 3342336 bytes)
========= [SUCCESS] Took ... seconds =========
```

**The two denominators must read `327680` and `3342336`.** The numerators (`30848`,
`283057`) are what the linker computes and they are the same on every desk in this room —
which is why they are allowed to be printed here at all. They are the only numbers in this
week's materials that are not yours.

> **📷 SCREENSHOT — NOT CAPTURED YET**
> `pio-01-build-sketch-e.png` — PlatformIO terminal, `[SUCCESS]` with both the `RAM:` and
> `Flash:` lines, denominators legible.

In the monitor, in this order:

1. About two seconds of nothing.
2. Five or six lines beginning with `#`, ending with
   `# ---- raw codes follow, one per line ----`.
3. **Then about eight more seconds of nothing.** This is normal — see below.
4. Two thousand integers, one per line, going past fast.
5. Three more `#` lines, the last of which is `# done`.

> **📷 SCREENSHOT — NOT CAPTURED YET**
> `serial-01-sketch-e-running.png` — Serial Monitor mid-dump: the `#` banner lines at the top
> and the raw codes streaming below.

`adcstats.py` prints eight aligned lines:

```
file                   w04/gnd-12db.txt
n                      2000
mean          (codes)  ____
stdev         (codes)  ____
min / max     (codes)  ____ / ____
peak-to-peak  (codes)  ____
noise-free bits        ____
bits from DC noise     ____
```

**`n` must read `2000`.** That is the check that the whole run reached the file.

### If it does not look like that

- **A denominator that is not `327680` / `3342336`** — most often `8388608`. You picked a
  different board or a different partition table. Stop, fix `platformio.ini`, rebuild. Do not
  carry on; every byte count you record today would be meaningless.
- **The monitor is blank for the first two seconds.** That is `delay(2000)`, deliberately
  there to let the monitor attach. Not a hang.
- **`# ---- raw codes follow ----` appears and then nothing happens for eight seconds.** Also
  not a hang. Sketch E captures all 2000 samples into RAM *first* and prints them afterwards —
  2000 samples at 250 Hz is exactly 8 seconds. Wait.
- **No `platformio-device-monitor-*.log` anywhere.** You did not overwrite the generated
  `platformio.ini`, so `monitor_filters = log2file` is missing. Fix it and take the run again.
  Do **not** rescue this by selecting text with the mouse — that is how lines go missing
  silently, and it is the same yes/no that mattered in W03.
- **`n` is not 2000.** Either you grabbed an older log, or the monitor was opened after the
  dump had already started. Reset the board, reopen the monitor, and take the newest log.
- **The pin is jumpered to GND and the readings are in the thousands.** The jumper is not
  actually in a GND pin. Check the silkscreen again before you blame the converter.
- **`adcstats.py` says `fewer than 2 numeric lines`.** Wrong file, or the run did not finish.

**→ Record sheet:** section 1, **row 1** — mean, std dev, peak-to-peak, noise-free bits.
Also the **DC offset** cell ("the mean of run 1, with the pin at 0 V", in codes) and the
**Run 1 bits from DC noise** cell. Leave "the same offset in millivolts" until run 2 has
printed you a scale factor — one run away, not one cycle away.

---

## Run 2 · Nothing on A0 — and the one number the rest of the morning needs

### Do this

1. **Take the jumper out of the board completely.** A0 is connected to nothing.
2. In `src/main.cpp`, change one line:
   ```cpp
   #define BIAS       1
   ```
3. **Rebuild and re-upload.** `BIAS` only exists at compile time, so a reset will not do it.
   ```bash
   python -m platformio run --target upload
   ```
4. Open the monitor, **then press RST** if it has not started on its own. That order matters —
   the board dumps all 2000 lines in a couple of seconds and anything printed before the
   monitor attaches is gone. Wait for `# done`, then stop the monitor. Rename the newest log
   `mid-12db.txt` and move it into `w04/`.
5. Check the banner says `# input bias : internal pull-up + pull-down ON (nothing on A0)`.
   If it says `OFF`, step 2 did not take — go back and rebuild.
6. **Copy the banner's scale-factor lines into the record sheet, character for character.**
   Either it computed:
   ```
   # mV and raw code  : ____._ mV, code ____._ (two separate reads)
   # scale factor     : _.____ mV per code, at that one point
   # the same x 4095  : ____ mV - extrapolation, not a measured full scale
   ```
   or it refused:
   ```
   # scale factor     : NOT COMPUTABLE - the input is sitting on a rail.
   ```
   **Copy whichever you got. The refusal is the entry, not a missing entry.**
7. ```bash
   python scripts/adcstats.py w04/mid-12db.txt
   ```

### You should see

The same banner shape as Run 1, with different numbers in it, and the same eight-line
`adcstats.py` block with `n  2000` — **and this time a `mV per code`**, because the pin is not
on a rail.

Unlike Run 1, this run has real noise in it: a standard deviation of about a code, a
peak-to-peak of a dozen or so, and therefore two different bit counts that both mean
something. Those are the numbers section 2 of the record sheet asks for.

**Why not just wire A0 to 3V3?** Because 3.3 V is outside what this converter can measure at
**every** one of its four attenuation settings — the widest of them still stops short of it,
and above the limit the reading is undefined. There is no setting that makes the supply rail a
usable reference point. Where those four limits are stated, and where they are not, is cycle
3's business.

### If it does not look like that

- **Nothing appears in the monitor at all.** You moved the jumper but did not press RST. All
  of Sketch E's work happens in `setup()`, and `loop()` is empty — once it has finished it will
  not run again until you reset it.
- **The banner still says `# input bias : OFF`.** You changed the `#define` but only pressed
  RST. Rebuild **and** re-upload, then take the run again.
- **The banner says `# scale factor : NOT COMPUTABLE` here.** Then something is still holding
  A0. Check the jumper is out of the board entirely — a wire left in A0 will be pulled around
  by whatever it touches and will swamp two resistors that are inside the chip. If the board
  is bare and it still refuses, call a TA: that is worth knowing about.
- **The reading is around 300 codes and jumping about by tens.** That is a floating pin, not a
  biased one. It means the pulls are not on. Same fix: rebuild and re-upload.
- **`analogReadMilliVolts()` looks absurd** — a few hundred millivolts with the pin hard at
  GND in Run 1, or a value that does not move when you move the jumper. **Tell a TA
  immediately.** The chip's calibration may not be burned on your unit, and that changes what
  you write in the record sheet.
- **You are tempted to copy a neighbour's `mV per code` because yours is missing.** Do not.
  A blank cell with a reason beside it is a result. A borrowed one is not yours to report.
- **You reflashed instead of resetting.** No harm done, you just spent two minutes.

**→ Record sheet:** section 1, **row 2**, and the **scale-factor block**, verbatim. This is
the run that fills "scale factor" and "extrapolated full scale". Section 2's two bit counts
come from this run as well — Run 1 was railed and has no noise to measure.

> Once you reflash for Run 3, that banner is gone. Write it down before you touch anything.

---

## Run 3 · Same input, attenuation changed to `ADC_0db`

### Do this

1. **Change nothing on the desk.** A0 is still connected to nothing, `BIAS` is still `1`, and
   the two resistors holding the pin are the same two resistors. One line of code is the only
   difference between this run and the last one.
2. In `src/main.cpp`, change that line:
   ```cpp
   #define ATTEN      ADC_0db
   ```
3. **Rebuild and re-upload.** A reset is not enough this time — you changed a `#define`, which
   only exists at compile time.
   ```bash
   python -m platformio run --target upload
   ```
4. Open the monitor, **then press RST**. Wait for `# done`, then stop the monitor.
5. Rename the newest log `mid-0db.txt`, move it into `w04/`.
6. Run all three of cycle 1's statistics together, so you can see them next to each other:
   ```bash
   python scripts/adcstats.py w04/gnd-12db.txt
   python scripts/adcstats.py w04/mid-12db.txt
   python scripts/adcstats.py w04/mid-0db.txt
   ```

### You should see

Three `adcstats.py` blocks, one after another.

**Read the third one all the way to the bottom before you copy anything out of it.**
`adcstats.py` prints its statistics first and its objections afterwards, and the third run may
print extra lines beginning `NOTE:` or `WARNING:` that the first two did not. Those lines are
part of the result. They are not decoration and they are not an error in the script.

Sketch E's own banner may also differ from Run 2's. Whatever it says, write it down.

`ADC_0db` is the setting with the **smallest** measurable range of the four. The voltage on
A0 did not change — run 2 measured it and gave you a number — but it is above what this
setting can see, so the same input now reads as a rail.

**That is the whole lesson of this run.** The attenuation setting is a claim about what is
allowed at the input, not a knob that scales the output. Nothing on the desk moved; one
`#define` decided whether the measurement was valid.

> **📷** A TA capture of this exact three-way comparison exists, but it shows the computed
> values, so it is not in the deck and it is not here — you are meant to produce your own.

### If it does not look like that

- **The third run's numbers are identical to Run 2's.** You changed the `#define` but only
  pressed RST. Rebuild **and** re-upload, then take it again.
- **You took the jumper out, or put one back in, between runs 2 and 3.** Then the two runs no
  longer share an input and the comparison is gone. Redo run 2 with the board bare.
- **`adcstats.py` printed a `NOTE:` or `WARNING:` and you are about to ignore it and copy the
  numbers above it.** Don't. Read section 1's "Run 3 — what happened, in one sentence" row of
  the record sheet before you decide what belongs in the table.
- **A number the tools would not give you.** Write down *what* was refused and *why*. Do not
  reconstruct it by hand — an invented number is the one thing this week marks as wrong.
- **You tried to type `ADC_12db` because the filenames say `12db`.** Type only what step 2
  gives you. The pack's README explains the naming: the Arduino core still exposes the old
  `ADC_11db` spelling, and the filenames are named for what the setting actually is. Both are
  correct; only one of them compiles.

**→ Record sheet:** section 1, **row 3** — with the caveat above. And **"Run 3 — what happened,
in one sentence"**, which asks you for the refusal, not for a value.

---

## Run 4 · A wire is an antenna

### Do this

1. Put `ATTEN` back to `ADC_11db` **and `BIAS` back to `0`**. **Rebuild and re-upload.**
   `BIAS` has to go back: two resistors inside the chip would hold the wire steady and the
   mains signal you are here to find would disappear into them.
2. Plug a **20 cm** jumper into **A0** and leave the far end connected to nothing at all.
3. Open the monitor and let it run once with the far end just **hanging**, touching nothing.
   Wait for `# done`.
4. **Stop the monitor.** This matters: the log is only closed when the monitor is, and two
   board runs inside one monitor session end up in one file. Rename this log
   `wire-hanging.txt` and keep it in `w04/` — it is not a required deliverable, but it is the
   other half of the comparison and it costs nothing to keep.
5. **Take hold of the far end of the jumper between finger and thumb first.** Then open the
   monitor. If the board does not start on its own when the port opens, press **RST**. Keep
   hold of the wire for the whole eight seconds, until `# done`.
6. Stop the monitor. Rename **that second** log `wire-touch.txt`, move it into `w04/`.
7. Run `hum.py`, passing **your own** scale factor — the one run 2 printed — as the second
   argument. Without it you get codes; with it you get microvolts, and microvolts is what the
   record sheet asks for.
   ```bash
   python scripts/hum.py w04/wire-touch.txt            # answer in codes
   python scripts/hum.py w04/wire-touch.txt 0.____     # answer in codes and microvolts
   ```
   Use **your** number, not your neighbour's. The `.png` is the same either way.

### You should see

```
file                   w04/wire-touch.txt
n                      2000  (8.0 s at 250 Hz)
peak in 55-65 Hz       ____ Hz
its amplitude  (codes) ____
broadband 80-120 Hz    ____ codes (median)
mains above the floor  ____ dB
mains amplitude        ____ uV
  for scale: an ECG R wave at the skin is about 1000 uV, before any amplifier
wrote                  w04/wire-touch.txt.png
```

and a new file `w04/wire-touch.txt.png` — a spectrum with a dashed red line at 60 Hz, on
**fixed axes** so that your plot and the one at the next desk are comparable.

**`n` must read `2000  (8.0 s at 250 Hz)`.**

**Nobody knows what this room will give you.** The deliverable is not a threshold you have to
beat. It is the numbers this room hands you today, and the ratio between hanging and held.

### If it does not look like that

- **`ModuleNotFoundError: No module named 'numpy'`.** `adcstats.py` is standard library, so
  everything above still stands — but `hum.py` is not. Get a TA; it takes five minutes.
- **A traceback ending in `FileNotFoundError`.** `hum.py` does not check the path politely.
  You mistyped the filename, or you never moved the log into `w04/`. List the folder and look.
- **`n` reads about `4000`.** Both the hanging run and the held run are in the same file,
  because the monitor stayed open across the reset. Take the held run again on its own.
- **`n` reads `2048` instead of `2000`.** You pointed it at a cycle-2 sine file. `hum.py`
  assumes 250 Hz and is hard-coded to it; give it a cycle-1 file. This is the signal contract
  from the README doing exactly what it warns you it will do — the rate is not in the file, and
  the tool will run happily at the wrong one.
- **You left the second argument off.** The script says so, tells you what it did instead, and
  gives you the answer in codes. That is still an answer, but you do not need to settle for
  it: run 2 printed your scale factor. Re-run the line with it.
- **The second argument is your neighbour's number.** It is not a class constant. It is yours.
- **The 60 Hz peak is barely above the broadband floor.** That is a result. Write down what you
  got and report the ratio. Nothing in this week promises the peak will be large.
- **Your laptop charger is plugged into the wall.** Write that down next to the numbers — it is
  a variable, and knowing that it was there is worth more than a tidy result that hides it.

**→ Record sheet:** section 1, **row 4**. All of section 2 — peak frequency, amplitude in
codes, amplitude in µV, broadband floor, mains above the floor in dB, then the three-cell
comparison block (1 mV in codes, 60 Hz amplitude in codes, ratio) and the sentence under it.

---

## End of cycle 1 — Lab Request 1

Before the first break, `w04/` should contain:

- [ ] `gnd-12db.txt`, `mid-12db.txt`, `mid-0db.txt`, `wire-touch.txt` — 2000 raw codes each
- [ ] `wire-touch.txt.png` — fixed axes, not adjusted
- [ ] Record sheet sections 1 and 2 filled, with every banner line copied verbatim
- [ ] **Every refusal written down**, in your own words, and never a reconstructed number
- [ ] Section 2 complete, microvolts included — run 2 gave you the scale factor it needs

---

# Cycle 2 — a sine you made yourself

**Two runs of Sketch F. Budget: 22 minutes of hands-on. This is the block most likely to
overrun, so keep moving.**

The sketch does not change between the two runs. **The breadboard does.**

---

## Run 5 · Build the RC, config A, and measure the chain's ENOB

### Do this

**Wire it first, with the USB unplugged.**

```
GPIO2 ---[ R ]---+--- GPIO1 (A0)
                 |
               [ C ]
                 |
                GND
```

0. Unplug the USB. Take the 20 cm jumper out of A0 — nothing from cycle 1 stays on the board.
1. Jumper from **GPIO2** to a free breadboard row. Call it row X.
2. **R** (10 kΩ, brown black orange) from row X to another free row. Call it row Y.
3. **C** (100 nF, the disc marked `104`) from row Y to the ground rail.
4. Jumper from the ground rail to a **GND** pin on the board.
5. Jumper from row Y to **GPIO1**. That is the whole circuit.
6. **Check before power.** GPIO2 and GPIO1 are adjacent on the right-hand header. Nothing
   connects to 3V3. Nothing connects to 5V. Neither component has a polarity, so neither can
   be in backwards.
7. Copy `sketches/sketch_f_sine.cpp` into `src/main.cpp`. Build and upload.
8. Open the monitor, **then press RST**. Wait for `# done`, then stop the monitor.
9. Rename the newest log **`sine-10k-100n.txt`** and move it into `w04/`. The resistor value is
   in the filename because it is nowhere in the file.
10. ```bash
    python scripts/enob.py w04/sine-10k-100n.txt
    ```

### You should see

The same two denominators in the build, with Sketch F's own numerators:

```
RAM:   [=      ]   9.5% (used 31240 bytes from 327680 bytes)
Flash: [==     ]   8.6% (used 286309 bytes from 3342336 bytes)
```

> **📷 SCREENSHOT — NOT CAPTURED YET**
> `pio-02-build-sketch-f.png` — PlatformIO terminal, `[SUCCESS]` for Sketch F, RAM and Flash
> lines legible.

In the monitor: banner lines, then a short pause, then
`# ---- raw codes follow, one per line ----`, then about two seconds of nothing while it
captures, then 2048 integers, then three closing `#` lines ending in `# done`.

**Two of those banner lines are about the carrier, and they are worth reading slowly:**

```
# carrier the divider implies : 1220 Hz (asked for 1220 - computed, not measured)
# the one-bit stream is 256x that : about 312320 Hz - that is the edge rate
```

`sigmaDeltaSetup()` does not measure anything. It returns the APB clock divided by
`(prescaler + 1) x 256` — arithmetic its own driver did, printed back at you. The number that
matters for the RC you just built is the second one: the modulator's output is switching at
roughly 312 kHz, and that is what your filter has to smooth away.

The 1220 Hz figure is the rate at which the whole 256-step pattern repeats. Your RC corner is
around 159 Hz, so 1220 Hz is only about eight times above it — some of that ripple survives the
filter and ends up inside the noise you are about to measure. **That is one of the reasons the
ENOB you get belongs to the chain and not to the converter.**

Two more banner lines are shouting at you in capitals, and they are right to: the sketch
cannot know which resistor you plugged in, so **the resistor and the capacitor have to be in
the filename**. That is what step 9 is for. Two files with the same name and different
components is the one mistake this week that cannot be recovered afterwards.

`enob.py` prints:

```
file                    w04/sine-10k-100n.txt
n                       2048  (2.048 s at 1000 Hz)
min / max       (codes) ____ / ____
peak-to-peak    (codes) ____  = ____ % of full scale
fundamental at bin      32  = 15.6250 Hz
fundamental amplitude   ____ codes peak
SINAD of the chain (dB) ____
ENOB of the chain(bits) ____
worst harmonic          __x fundamental, ____ dB below it

This is the ENOB of the whole chain: sigma-delta generator -> your R and C
-> the ADC. The generator's quantisation and the filter are counted as
error along with the converter's, and this rig cannot separate them.
ENOB is only comparable between two measurements taken at the SAME input
amplitude. Yours filled ____ % of full scale. Write that down next to the
number, or the number cannot be compared with anybody else's.
wrote                   w04/sine-10k-100n.txt.png
```

**Read the words "of the chain" and take them literally.** The tone was made by the modulator,
smoothed by your R and C, and only then read by the ADC — so the generator's own quantisation
and whatever your filter did are inside the figure. Separating out the converter alone would
need a signal source known to be better than the converter, and this rig has no such thing.
That is not a weakness in the exercise: what a signal chain can resolve is the question the
error budget in cycle 3 actually asks.

Three things to check, in this order, before you write anything down:

1. **`n` reads `2048`.**
2. **`fundamental at bin` reads `32`.** That is not a measurement, it is arithmetic: 2048
   samples ÷ 64 points per cycle = 32 whole cycles in the record.
3. **No `*** CLIPPED` block.**

### If it does not look like that

- **The readings are flat, or `enob.py` says `every sample is identical. There is no sine
  here`.** In order of how often it happens: (a) the capacitor is on the other side of the
  breadboard's centre channel, so it is not actually in row Y — a breadboard row is five holes
  and the gap in the middle is a break, not a connection; (b) the ground rail never gets back
  to a GND pin on the board; (c) you counted the header wrong and are driving something that is
  not GPIO2.
- **The banner's `# scale factor` line says `NOT COMPUTABLE` here too.** Then the node is not
  at mid-scale, which means the circuit is wrong, not the chip. Check the three things in the
  next bullet — the capacitor's row, the ground, and which pin is being driven — and take the
  run again. **Get a TA if it still refuses**; you do need this number.
- **`*** CLIPPED` appears.** Write it down and say so. **Tell a TA — do not fix it at your own
  desk.** A clipped ENOB is a measurement of your input range, not of your chain, and the
  fix has to be the same for all forty-five people.
- **`fundamental at bin` is anything other than 32, and `enob.py` prints a `NOTE` about it.**
  Stop and get a TA. Either the capture is not coherent or the file is not what you think it
  is, and every number below that line is untrustworthy.
- **`n` reads `2000`.** You pointed `enob.py` at a cycle-1 file. Cycle 1's files are 250 Hz
  and 2000 samples; cycle 2's are 1000 Hz and 2048. The script will happily produce a full set
  of numbers from the wrong file — the `NOTE` about the bin is your only warning.
- **`ModuleNotFoundError: No module named 'numpy'`.** This is the one step today that numpy
  actually blocks. Get a TA.
- **Build fails.** Two usual causes: the `#include <Arduino.h>` at the top of the file was lost
  when you copied it, or there are now two files in `src/` that both define `setup()`.
- **`# worst sampling interval error` is a large fraction of 1000 µs.** Tell a TA. The whole
  coherence claim rests on that clock, and the sketch prints the evidence for it on purpose.

**→ Record sheet:** section 3, **row A** — peak-to-peak in codes, % of full scale, chain SINAD,
chain ENOB. And in the block below it, config A's **fundamental amplitude** and **worst
harmonic**, plus the **worst sampling-interval error** from the banner.

**→ Also, now:** section 1's **scale factor**, **extrapolated full scale** and **DC offset in
millivolts**, from this run's banner — and Run 4 step 7's one `hum.py` line, re-run with your
scale factor, which fills section 2's µV cell.

---

## Run 6 · Swap to config B, and decide which number you would defend

### Do this

1. **Unplug the USB.** Take out the 10 kΩ and the `104`. Put them back in the bag.
2. Put in the **100 kΩ** (brown black yellow) and the **`103`** disc, in exactly the same two
   places. Nothing else moves.
3. **Say out loud to the person next to you which resistor is on your board right now.** The
   file cannot record it, and in four minutes you will not remember either.
4. Plug the USB back in. **Open the monitor**, *then* press **RST**. **Do not reflash** — not
   one byte of the firmware changed, and reflashing costs two minutes each time.
5. Wait for `# done`, then stop the monitor.
6. Rename the newest log **`sine-100k-10n.txt`**, move it into `w04/`.
7. ```bash
   python scripts/enob.py w04/sine-100k-10n.txt
   ```
8. Decide which of the two runs you would quote as your **signal chain's** ENOB. Then **copy** that file —
   copy, not rename; you need both originals as well:
   ```bash
   cp w04/sine-10k-100n.txt w04/sine.txt          # macOS, Linux, Git Bash, PowerShell
   copy w04\sine-10k-100n.txt w04\sine.txt        # Windows cmd.exe
   ```
   Either file is a defensible choice. What is marked is the reason, not the pick.
   **W06 filters `sine.txt` in two weeks.** If it is missing, W06 has nothing to work on.

### You should see

The same nine-line `enob.py` block, from a different file, with `n  2048` and
`fundamental at bin  32` again.

> **📷 SCREENSHOT — NOT CAPTURED YET**
> `enob-01-two-configs.png` — terminal: both `enob.py` outputs side by side, with the
> amplitude percentage and the ENOB line visible in each.

**Read the amplitude percentage before you read the ENOB, both times.** Two ENOB figures taken
at different amplitudes are not comparable, and half the room will have different amplitudes.

### If it does not look like that

- **The two ENOB figures are identical to two decimal places.** Check the obvious thing first:
  are the components actually swapped? Two identical runs are the usual signature of parts that
  never moved. If they genuinely did move, then you measured a null — **that is a result, and
  the record sheet has a line for it.** Say so, and say what it would take to detect a
  difference you could not see.
- **You are about to claim config B is worse (or better).** Did you repeat either one? A
  difference smaller than the spread between two repeats of the *same* setup is not an effect.
  Nobody in this room has that repeat, which is exactly why the record sheet asks you whether
  you did.
- **Two files ended up with the same name.** This is the one mistake this week that cannot be
  recovered after the fact, because nothing inside either file says which resistor was fitted.
  Take both runs again.
- **You reflashed at step 4.** No harm, two minutes gone.
- **You have not made `sine.txt`.** Do it now, before you pull the breadboard apart. It is on
  the "before you leave" list for a reason.

**→ Record sheet:** section 3, **row B**, and config B's column in the block below it. Then the
prose answers: what changed between A and B; the ENOB difference in bits; whether you repeated
a configuration; and **which of the two you would quote as your signal chain's ENOB, and why**. That
last one is a judgement, not a maximum.

---

## End of cycle 2 — Lab Request 2

Before the second break:

- [ ] `sine-10k-100n.txt` and `sine-100k-10n.txt` in `w04/` — 2048 raw codes each, at 1000 Hz
- [ ] Both `.png` spectra from `enob.py`, axes not adjusted
- [ ] `sine.txt` — a copy of the run you would defend
- [ ] Record sheet section 3, with **each chain ENOB carrying its amplitude as a percentage of
      full scale**. An ENOB without its amplitude is not a measurement.

**One more thing before you go to the break.** Do **steps 1 and 2 of Run 8** now — send the ADC
prompt and paste the answer into `AI_USAGE.md` unedited. The checking happens after the break,
and it only works if the answer was obtained before you read the primary source.

---

# Cycle 3 — what your number means for a 1 mV ECG

**No hardware. Budget: 19 minutes of hands-on.** Leave the breadboard alone; you are not done
with it until you have checked the list at the end of this guide.

---

## Run 7 · The error budget

This is arithmetic on rows you have already filled, plus two numbers from W03. Show the
arithmetic — the record sheet has space for it.

### Do this

1. Open your **`w03/RECORD_SHEET.md`** and find the standard deviation of your sampling
   interval for **both** the timer version and the `delay(4)` version, in milliseconds.
2. Fill section 4 of today's sheet with both, and with which one you would use.
3. Compute your **effective LSB**, in microvolts:
   `effective LSB = (your extrapolated full scale in mV) ÷ 2^(your chain ENOB)`
4. Do it **again** with 12 in the exponent instead of your chain ENOB. The first is what the word
   length promises; the second is what you measured. The ratio between them is the price of
   reality, and you can defend it because you measured the exponent yourself.
5. Compute how many effective codes a 1 mV R wave occupies: `1 mV ÷ (effective LSB)`.
6. Choose a frequency *f* for the jitter term, and **write down why you chose it**. The ECG
   band does not have one frequency. The QRS band, the 150 Hz diagnostic limit and the 40 Hz
   monitoring bandwidth are all defensible; picking one and owning it is the assignment.
7. Compute `ΔV(rms) = 2π · f · A · σt` with `A = 1 mV` and σ*t* from step 1 — **in seconds**.
8. Express ΔV in your own effective codes, and subtract it from step 5.
9. Write the one-sentence answer, **with your own numbers inside it**.

### You should see

Every cell in section 5 filled with a number that came from a row above it, and a final
sentence that contains at least three of your own figures.

### If it does not look like that

- **You cannot find your W03 numbers.** Write **"not recovered"** and say what you tried. Do
  not leave the row blank. **Then tell a TA now** — there is a way to recover it and it is much
  easier at 09:15 than at 11:45.
- **σ*t* is in milliseconds and the formula wants seconds.** This is the commonest slip in the
  whole section, and it puts your answer out by a factor of a thousand in a direction that
  looks plausible.
- **Your chain ENOB was measured at a low percentage of full scale.** The division is still valid.
  Note the percentage next to it — the comparison with anybody else's is what stops being
  valid, not your own arithmetic.
- **Your answer is "it is not good enough".** That is a mood, not a result. The sentence has to
  say *by how much*, in your units.

**→ Record sheet:** all of section 4. All of section 5 — steps 1 through 4, including the
frequency you chose, your reason, σ*t* in seconds, ΔV in µV and in effective codes, and the
final sentence. Also section 7, the method note.

---

## Run 8 · The AI unit, part one — the converter you are holding

**Send the prompt before the break. Check it afterwards.** That order is not politeness, it is
experimental design: if you read the primary source first you will grade the answer against
your memory instead of against the document.

### Do this

1. Send this, verbatim, to whichever assistant you use:

   > Summarise the ESP32-S3 ADC for a biomedical data logger: the resolution, the four
   > attenuation settings and the input voltage range each one gives, the absolute accuracy,
   > and the input impedance of an ADC1 pin. Give a source for each figure.

2. Paste the answer into `AI_USAGE.md` **unedited**, now.
3. After the break, find the attenuation table it gave you and put **your own full scale from
   Run 2** next to it. Write down the difference **in millivolts**. Not "close" — a number.
4. Open at least one of these three files. They are already on your laptop, inside your
   PlatformIO install under `packages/framework-arduinoespressif32/`:
   ```
   tools/sdk/esp32s3/include/soc/esp32s3/include/soc/soc_caps.h      SOC_ADC_MAX_BITWIDTH
   tools/sdk/esp32s3/include/hal/include/hal/adc_types.h             the attenuation enum
   cores/esp32/esp32-hal-adc.c                                       DEFAULT_VREF
   ```
5. For each claim, write down where the vendor actually states it — or that it is **not
   located** in any of the three.
6. Fill the four fields of entry 1 in `AI_USAGE.md`: what I asked · what it answered · **how I
   checked** · what I changed.

### You should see

An answer that is confident, cleanly formatted, and gives voltage ranges to three digits — and
three header files that between them do not say everything the answer said.

### If it does not look like that

- **It gave no source for anything.** That *is* your finding, and it goes straight into the
  "how I checked" field. It is a complete answer to the question that was asked.
- **You cannot find one of the three files.** Search your PlatformIO install for the filename
  rather than the whole path; installs differ. If it is genuinely absent, say so — that is also
  a finding.
- **You are about to write "it looked right" in the "how I checked" field.** That scores
  nothing. So does "it matched what I remembered". Name a file and a line, or write **"not
  located"** and say what you searched for.
- **You only read this guide and the slide, and did not open a file.** Reading about a check is
  not a check.
- **You are about to write that the model lied.** Nobody is claiming that, and most of the
  table may well be right. The point is that you cannot tell the right rows from the wrong ones
  by looking, because they are formatted identically.

**→ Record sheet:** nothing in the sheet itself. This produces **entry 1** of `AI_USAGE.md`.

---

## Run 9 · The AI unit, part two — the chip we could not buy

### Do this

1. Open the **AD8232 data sheet** you downloaded before class. Offline. Do not download it in
   the room — forty-five people on the same wifi is not a plan. If you do not have it, pair up
   with someone who does.
2. **Before reading it**, send this:

   > I am specifying a single-lead ECG front end using the AD8232. Give me its gain, its
   > common-mode rejection ratio, its bandwidth, and what the right-leg drive output is for.
   > Cite the page or table for each.

3. Put the answer on one side of your screen and the data sheet on the other.
4. For each of the four items, find it in the document and write down **the page, table or
   section number**. If it is not there, write **"not located"** and what you searched for.
5. Fill the four fields of entry 2 in `AI_USAGE.md`. The "how I checked" field must name a page
   or a table.

### You should see

Two windows, and four rows in section 6 whose third column contains page numbers or the words
"not located" — nothing else.

> **📷 SCREENSHOT — NOT CAPTURED YET**
> `ai-01-datasheet-side-by-side.png` — two panes: the assistant's four-item answer on the left,
> the data sheet open at the page being checked on the right.

### If it does not look like that

- **The download link 404s.** Search Analog Devices' own site for AD8232 and take the data
  sheet from there. **Do not take a copy from a file host.** The whole exercise is whether a
  claim can be traced to the manufacturer's document, and a PDF of unknown provenance cannot do
  that job.
- **A figure is quoted with no conditions attached.** A rejection ratio is stated at a
  frequency and a source imbalance; the same part has different numbers at different
  conditions. A figure with no condition cannot be verified *or* falsified, which makes it the
  most dangerous kind of statement in the answer. Note the mismatch.
- **Right-leg drive is the one you are about to skim.** A summary finds it easy to say what it
  *is* and easy to skip what it is *for*. Read that section properly.
- **All four check out.** Then say so, and say how long it took. "All four located, twelve
  minutes" is a result — it is evidence about how good an index the tool is, which is exactly
  what this week claims it is for.
- **You wrote "datasheet" in the third column with no page number.** That is not a check.

**→ Record sheet:** all of section 6 — four rows, third column filled or marked "not located";
the "anything the AI stated that you could not locate" line; and the one sentence on what a
dedicated front end sells you that your board does not have, in terms of your sections 1, 3 and
5. Plus **entry 2** of `AI_USAGE.md`.

---

# Bonus · not run in class

Two separate bonus routes. They are not the same thing, and the record sheet only has a table
for the second.

## B1 · Sketch G — is the curve straight?

A standard deviation taken at one input tells you nothing about the *shape* of the transfer
curve, and averaging does not fix a bend. Sketch G parks the sigma-delta at 17 fixed duty
values, waits for your RC to settle, and takes 200 readings at each.

### Do this

1. Leave the breadboard exactly as it is — config A. No new parts.
2. Copy `sketches/sketch_g_steps.cpp` into `src/main.cpp`. Build and upload.
3. Open the monitor, **then press RST**. Wait for `# done`, then stop the monitor. Rename the
   log `steps.txt`, move it into `w04/`.
4. ```bash
   python scripts/linfit.py w04/steps.txt
   ```

### You should see

The file is **two columns**, `duty,code`, not one integer per line — this is the only file
today that breaks the signal contract, on purpose, because it carries a second variable.

```
file                    w04/steps.txt
steps used              __ of 17
best straight line      code = ____ * duty + ____
worst departure  (codes) ____
  as a fraction of FS    ____ %
  bits left if this bend ____ noise-free bits - i.e. the bend alone caps you here
monotonic              yes
within-step stdev       ____ .. ____ codes
wrote                   w04/steps.txt.png
```

### If it does not look like that

- **`found 0 steps. Sketch G writes 17.`** You pointed it at a one-integer-per-line file from
  cycle 1 or 2. Point it at `steps.txt`.
- **`dropping N railed step(s) from the fit`.** A railed step is not a point on the curve, it
  is the end of it, and the script is right to leave it out. Note how many it dropped.
- **`NON-MONOTONIC`.** Before blaming the converter, check `SETTLE_MS` (120 ms in the file)
  against your own R × C. If the settling is too short, every step carries the tail of the one
  before it and what you measured is your own impatience.

**→ Record sheet:** no cell. This is material for the HW2 bonus on nonlinearity.

## B2 · Raise your chain's ENOB

Knobs that cost no new parts: the attenuation setting, the sample rate, oversampling and
averaging, the R and C values, the sigma-delta carrier frequency.

Re-run Sketch F with **one** knob changed, and name the price you paid. Note which part of the
chain the knob is in: some of these improve the generator, not the converter, and saying which
is part of the answer. Oversampling buys bits
with bandwidth. Averaging buys them with latency. **A bigger number with no named cost scores
nothing.**

**→ Record sheet:** section 8 — ENOB and % of full scale before and after, and what it cost.

---

# Before you leave

From the record sheet's own list. Check it against `w04/`:

- [ ] `gnd-12db.txt`, `mid-12db.txt`, `mid-0db.txt`, `wire-touch.txt`
- [ ] `wire-touch.txt.png`
- [ ] `sine-10k-100n.txt` and `sine-100k-10n.txt`, plus both `.png` spectra
- [ ] **`sine.txt`** — the run you would defend. W06 filters this one.
- [ ] `scripts/` committed, so your numbers can be reproduced
- [ ] Two entries in `AI_USAGE.md`
- [ ] `RECORD_SHEET.md`, filled in
- [ ] Google Form submitted **before you leave the room**

You cannot re-create any of this once the board is reflashed and the breadboard is pulled
apart. An empty cell you can explain is a result. A guessed cell is not.
