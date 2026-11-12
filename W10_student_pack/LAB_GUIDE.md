# W10 Lab Guide — run by run, at your own pace

**Embedded Systems (BME 3301) · Week 10 · Thursday 12 Nov, 09:10–12:00 · 醫技醫工館 5776**

> ## Where you put this folder can break the build
>
> **The path to your project must contain only plain ASCII — no Chinese characters — and must
> not be very deep.** Two real failures, both found by TAs on real machines:
>
> - **Chinese anywhere in the path.** The compiler writes a temporary file, cannot open it, and
>   stops with `Invalid argument` followed by `fatal error: no input files`. Nothing in the
>   message mentions the path. A folder like `下載\嵌入式系統\` is enough to cause it.
> - **A very deep path.** Windows limits a path to about 260 characters. Past that the build
>   fails with `No such file or directory` on files it just created.
>
> **Put your project somewhere short and English**, for example `C:\dev\bme3301\w06`.
> If you hit either error, move the folder before you change anything else — the code is fine.


> **Where the monitor log actually is.** When the monitor starts it prints
> `--- Logging an output to <path>`. **Read that line** — it is the only answer that is right
> on every machine. On PlatformIO Core 6.2 and newer the path is
> `logs/device-monitor-YYMMDD-HHMMSS.log` inside your project; on older Core versions it is
> `platformio-device-monitor-*.log` in the project folder itself. Do not go hunting for a
> filename somebody told you; the monitor already told you.


This is the document you drive yourself. The slides move at one speed for forty-five people;
this does not.

Every run has the same three parts:

| | |
|---|---|
| **Do this** | Numbered steps. One action each. Do them in order. |
| **You should see** | The *shape* of the output — the keys, the markers, the column names. **Never a value.** Every number today is yours. |
| **If it does not look like that** | The ways this step actually goes wrong, and what to do about each. Read this column **before** you raise your hand. |

Each run ends with **→ Record sheet**.

> **Four runs, and the fourth is the comparison.** Run 1 and 2 are two sessions of data off
> your board. Run 3 is a rule and a model, each scored on the session they never saw. Run 4
> is both of them on the board, on the same captured window, timed the same way.

> **Every output string quoted below was checked against the file that prints it.** If your
> screen says something this guide does not list, that is worth a hand in the air.

---

## Before you start — how the screenshots work

Ten reference frames belong in the deck. **None have been captured yet** — the TA dry run on
Wed 11 Nov produces them. Until then the frames in the deck read `SCREENSHOT — TA CAPTURING`.

---

## Run 0 · Set the project up

**Time: about 5 minutes** — you did most of it in the preflight.

### Do this

1. Unzip `W10_student_pack.zip` **next to** the earlier packs, not into them.
2. Open your `w10` PlatformIO project (preflight item 1). Confirm `src/` holds **three** files:
   `w10_imu.h`, `model_weights.h`, `main.cpp`.
3. Confirm the five blanks in `src/w10_imu.h` carry your W05 values (preflight item 2).
4. Make a folder `w10/` for everything you produce today — **inside the unzipped pack folder,
   next to `scripts/` and `sketches/`, not inside your PlatformIO project** (which is also
   called `w10`, so this is easy to get wrong). Every `python scripts/...` command today runs
   **from the pack folder**; each session log the monitor writes into the project's `logs/`
   gets renamed and moved into this `w10/`. If `windows.py` says a file does not exist, check
   which of the two `w10` folders you are standing in.
5. Fill record sheet **section 0** now — the two spec lines and the consent boxes. If you plan
   to swap sessions with a neighbour, both of you sign now, not later.

### You should see

A build that succeeds with the two familiar denominators, `327680` and `3342336`.

### If it does not look like that

- **`fatal error: w10_imu.h: No such file`** or the same for `model_weights.h`. Both headers
  are in the pack's `sketches/` folder and have to be in `src/` next to `main.cpp`.
- **It builds but `halt=blank_not_filled` prints at run time.** The blank it names is still
  `0x00` / `0.0f`. Your W05 record sheet section 3 has the four; the fifth is the gyroscope's
  sensitivity from the same datasheet table.

**→ Record sheet:** your name, the date, section 0.

---

## Run 1 · Session A

**Time: about 6 minutes.**

### Do this

1. `sketches/sketch_a_collect.cpp` is `src/main.cpp`. Build. **Write RAM and Flash down.**
2. Upload. Open the monitor at `115200`. **Pick up the breadboard first, then press RST.**
   The sketch does not wait for you: the first prompt arrives about 3 seconds after reset and
   the countdown runs whether your hands are ready or not. The TAs had to restart twice. If you
   miss the first few windows, press RST and start the session again — it costs three minutes,
   and a session with four guessed windows in it is worse than no session.
3. Read the first lines: `# who_am_i=` and the `# W10 collect` header.
4. Do what each prompt says, thirty-two times. Each window: the prompt, `>> 3`, `>> 2`,
   `>> 1`, `>> GO`, two seconds, `>> STOP - hold on, printing`. **About three minutes** (measured:
   3 min 1 s).
   **What has to move is the GY-521, not the DevKit.** Hold the breadboard the module is seated in (with the DevKit riding along if it is plugged into the same breadboard). If the DevKit sits on the desk on its own, that is fine — the four jumpers follow. Holding the DevKit while the breadboard stays flat records `still` for all four classes, and nothing will tell you until the feature table shows four identical columns.
5. Wait for `# done windows=32`. Close the monitor. Rename the newest log
   `sessionA.txt` and move it into `w10/`.

### You should see

```
# who_am_i=0x__ expected=0x__ addr=0x__
# W10 collect - one session
# source=mpu6050 addr=0x__
# fs_hz=100 window_samples=200 n_feat=8
# classes=still,tilt,shake,tap
# accel_cfg=0x__ xl_sens_mg=_.____ gyro_cfg=0x__ g_sens_mdps=__.____
# reps_per_class=8 windows=32
# columns: i,ax_g,ay_g,az_g,gx_dps,gy_dps,gz_dps
# Watch the screen. Each window: a prompt, 3-2-1, GO, two seconds, STOP.

>> window 1 of 32 : still - put the board flat on the desk and take your hand off it
>> 3
>> 2
>> 1
>> GO
>> STOP - hold on, printing
W 1 still begin
0,_.___,_.___,_.___,__._,__._,__._
...
199,_.___,_.___,_.___,__._,__._,__._
W 1 still end ticks=200 missed=0 clipped=0 read_failed=0
```

and at the end:

```
# done windows=32 still=8 tilt=8 shake=8 tap=8
# One run = one session. Save this log, then run it again for the other session.
```

**`missed=0 clipped=0 read_failed=0` on every end line is what you want.** A window that says
otherwise is not an error; `windows.py` will drop it and tell you why.

### If it does not look like that

- **Any `halt=` line, then silence.** The sketch refused. `halt=identity_read_failed`: nothing
  answered at 0x68 or 0x69 — W05 sketch 1, four wires. `halt=identity_mismatch`: the part is
  not what `WHOAMI_EXPECT` says. `halt=write_nacked step=N`: a configuration write was not
  acknowledged; a loose wire mid-run. `halt=no_data_ready_before_timeout`: the part woke but
  never flagged a sample — power-cycle it.
- **You only caught the tail of the header.** The monitor attached late. Press RST and start
  again; `windows.py` refuses a log without `fs_hz` in the header.
- **`clipped=` is not 0 on `tap` windows.** Something hit full scale. Tap more gently for the
  rest, and note it on sheet section 1. Do not change the range between sessions A and B — the
  two sessions must be recorded under the same settings.
  **It is not always the accelerometer.** `clipped` counts a sample where the accelerometer
  **or** the gyroscope is at full scale. A sharp tap spins the board as well as shaking it, and
  the TA dry run caught windows where `amag_max` was only 1.8–2.3 g — nowhere near the ±4 g
  limit — while `gmag_max` was 525–535 dps, past the ±500 dps gyroscope limit. So before you
  conclude anything about the accelerometer range, check `gmag_max` on the same window.
  Firm taps on the desk edge did not clip at all in the dry run; only harder ones did.
- **`missed=` is not 0.** The serial dump of the previous window ran into the next tick.
  This should not happen (the dump is before the countdown); tell a TA, with your laptop
  model, and continue — the window is dropped, the rest are fine.
- **No `logs/device-monitor-YYMMDD-HHMMSS.log`.** Same fallback as every week, from the PlatformIO terminal
  inside VS Code (the one where `pio` is on the PATH):
  `pio device monitor --raw > w10/sessionA.txt`, `Ctrl+C` after `# done`.

**→ Record sheet:** section 6 row "Sketch A — yours" (RAM, Flash). Section 1, column A, after Run 2b.

---

## Run 2 · Session B — a different sitting

**Time: about 5 minutes.**

### Do this

1. **Put the board down. Take your hand off it. Pick it up again.** This is not a ritual —
   it is the only thing that separates session B from session A, and slide 20 is about why.
2. Press **RST** with the monitor open. Thirty-two prompts again.
3. Wait for `# done windows=32`. Close the monitor. Rename the log `sessionB.txt` into `w10/`.
4. **Optional, and better:** swap `sessionA.txt` with a neighbour who signed section 0. Save
   theirs as `w10/neighbour_sessionA.txt`. Their session is your test set.

### You should see

The same shape as Run 1, ending in `# done windows=32 still=8 tilt=8 shake=8 tap=8`.

### If it does not look like that

- See Run 1. The same four `halt=` lines mean the same four things.
- **You forgot to rename session A first** and now have two logs. The newer one is B. Check
  the timestamps in the file names before you rename.

**→ Record sheet:** nothing yet; Run 2b fills section 1.

---

## Run 2b · From two logs to two sets

**Time: about 3 minutes.**

### Do this

1. ```bash
   python scripts/windows.py w10/sessionA.txt --test w10/sessionB.txt --out w10_out
   ```
   or, if you swapped: `--test w10/neighbour_sessionA.txt`.
2. Read the TRAIN and TEST blocks. Copy the per-class counts and every `dropped window` line
   into sheet section 1.
3. Copy the "a model that always says '____' would score ____%" line into section 1.

### You should see

```
TRAIN  w10/sessionA.txt  __ windows kept, _ dropped
         per class: still=_  tilt=_  shake=_  tap=_
TEST   w10/sessionB.txt  __ windows kept, _ dropped
         per class: still=_  tilt=_  shake=_  tap=_
         a model that always says '____' would score __._% on this test set

# feature table - TRAINING SESSION ONLY (sessionA). Choose T_TAP_G from amag_max,
# T_SHAKE_G from amag_std, T_TILT_DPS from gmag_mean. Do not look at the test session.
feature        still                     tilt                      shake                     tap
               min   median   max        ...
amag_mean      ...
amag_std       ...
amag_max       ...
...
WROTE  w10_out/features_train.csv, features_test.csv, feature_table.txt, manifest.json
WROTE  w10_out/ei_export/training (__ files), ei_export/testing (__ files)
```

A dropped window looks like:

```
         dropped window 17 (tap): 2 sample(s) at full scale: the peak is not the peak
```

### If it does not look like that

- **`REFUSED  no --test given.`** Read the paragraph; it is slide 20 in prose. You need a
  second session. There is no flag that makes it split one session at random.
- **`REFUSED  ... are the same file`.** You passed session A twice.
- **`REFUSED  the test set has too few windows for tap: tap=3. The minimum is 5 per class.`**
  Too many `tap` windows clipped or missed. Options, in order: use your neighbour's session as
  the test set; re-record session B tapping more gently; if the training set is the short one,
  re-record A. Say which you did on sheet section 1.
- **`REFUSED  ... is not a W10 collect log`.** You passed `live.txt` or a W06 file.
- **`REFUSED  ... has no fs_hz in its header`.** The monitor attached late on that session.
  Record it again.
- **`REFUSED  ... was generated by selftest.py (source=SYNTHETIC)`.** You passed a file from
  `selftest_out/`. Those are not data.
- **Every class has the same min / median / max on every row of the feature table.** The
  IMU did not move: you held the DevKit and the breadboard stayed on the desk. Record both
  sessions again, holding the breadboard.

**→ Record sheet:** section 1, both columns. Section 2, the table — **training session only**.

---

## Run 2c · Three numbers, and the rule's matrix

**Time: about 5 minutes.**

### Do this

1. From `w10_out/feature_table.txt`, choose `T_TAP_G` (from the `amag_max` row),
   `T_SHAKE_G` (`amag_std`), `T_TILT_DPS` (`gmag_mean`). Write them in section 2, with the
   min/max of each class on that row. **Do not open the test session's features while choosing.**
   **Two things the dry run found in this table, on real hardware:**
   - **Your `still` rows may not sit near zero on `gmag_mean`.** Every gyroscope has a
     zero-rate offset, and it is different on every module — the TAs' read about +35 dps on one
     axis while the board lay untouched, so their `still` windows had `gmag_mean` ≈ 35, not ≈ 0.
     Your `T_TILT_DPS` has to clear **your own** `still` rows, not zero. Read the number off
     your table; do not copy anyone else's.
   - **`tap` and `shake` can overlap on `amag_max`,** and no single threshold separates them
     when they do. That is not a mistake you made, and it is worth writing down rather than
     hunting for a better number: it is the clearest thing one feature and one threshold cannot
     do, and cycle 2 is about what happens when eight features are used at once.
2. ```bash
   python scripts/baseline.py w10_out --tap <T_TAP_G> --shake <T_SHAKE_G> --tilt <T_TILT_DPS>
   ```
3. Copy the **TEST** matrix, the recall column, the precision row, the overall accuracy and the
   always-majority line into section 3.

### You should see

```
RULE   tap: amag_max > _.___ g   shake: amag_std > _.___ g   tilt: gmag_mean > __._ dps

TRAINING session (the one you chose the thresholds from - expect this to look good)
  rows = what you did, columns = what it said   (n = __ windows)
  ...
TEST session (the one that counts)
  rows = what you did, columns = what it said   (n = __ windows)

         still  tilt shake   tap   recall   (granularity)
  still      _     _     _     _   __._%  (1/_ = __ pts)
  tilt       _     _     _     _   __._%  (1/_ = __ pts)
  shake      _     _     _     _   __._%  (1/_ = __ pts)
  tap        _     _     _     _   __._%  (1/_ = __ pts)
  precision  __%   __%   __%   __%

  overall accuracy        __._%
  always-'____' baseline  __._%   <- a model that never looks at the data scores this
  largest off-diagonal cell: did ____, said ____, _ time(s)

WROTE  w10_out/cm_rule.json  (the TEST matrix; compare.py reads it)
       Put the same three numbers into sketch_c_classify.cpp: T_TAP_G, T_SHAKE_G, T_TILT_DPS.
```

### If it does not look like that

- **`REFUSED  --tilt not given.`** All three, every time. There are no defaults, because a
  default would be a number you did not choose.
- **`REFUSED  --shake = 0.0. A threshold at or below zero fires on every window`.** Choose a
  number from the table.
- **The TRAINING block is perfect and the TEST block is not.** That is expected. The
  thresholds were chosen from the training session; the test session is the one that counts.
- **The TEST block has `n/a` in a precision column.** The rule never said that class. Not an
  error — it is a finding. Write it down.
- **`NOTE   source=SYNTHETIC` under the matrices.** You pointed `baseline.py` at a
  `selftest_out/` folder. Not data; use `w10_out`.

**→ Record sheet:** section 2 (your three numbers), section 3 (the TEST matrix and the lines under it).

---

## Run 3 · Train — Edge Impulse Studio, and the offline path

**Time: about 10 minutes, with the AI unit running while it trains.**

### Do this

1. **Everyone:**
   ```bash
   python scripts/train.py w10_out --header sketches/model_weights.h
   ```
   Copy the **TEST** matrix into sheet section 4 (the `train.py` table), with the epochs and
   seed it printed. Note the `bytes` figure for section 6.
2. **If you have an account and the network is cooperating:** follow `EDGE_IMPULSE.md`
   — upload `w10_out/ei_export/training/` as **Training**, `testing/` as **Testing**, create the
   impulse (2000 ms window, Spectral features, Classification), Generate features, Start
   training. While it trains: the AI unit, slides 28–33.
3. **Model testing → Classify all.** Copy the 4×4 (counts, rows in `still, tilt, shake, tap`
   order) into section 4's Studio table.
4. ```bash
   python scripts/compare.py w10_out/cm_rule.json w10_out/cm_model.json
   ```
   and, with the Studio matrix as well:
   ```bash
   python scripts/compare.py w10_out/cm_rule.json w10_out/cm_model.json \
          --manual "a,b,c,d;e,f,g,h;i,j,k,l;m,n,o,p" --manual-name edge-impulse
   ```
5. Fill section 5 — the last column (one window = how many points) **first**.

### You should see

From `train.py`:

```
MODEL  8 features -> 12 hidden (ReLU) -> 4 classes (softmax), 400 epochs, seed 0
       weights + standardisation = 176 float32 = 704 bytes (float32, unquantised - W11 starts here)

TRAINING session (fit quality only - this is not a result)
  ...
TEST session (the one that counts)
  ...
WROTE  sketches/model_weights.h  (MODEL_READY 1; copy it into src/ next to sketch_c_classify.cpp and rebuild)
WROTE  w10_out/cm_model.json  (the TEST matrix; compare.py reads it)
```

From `compare.py`:

```
== rule ==
  ...
== model-numpy ==
  ...
PER-CLASS RECALL   (of the windows where you did X, how many did it call X)
                      rule   model-numpy
still               __._%        __._%
...
PER-CLASS PRECISION (of the windows it called X, how many were X)
...
accuracy             __._%        __._%
majority             __._%        __._%

WHAT THE MODEL CHANGED, recall, model minus rule, on these __ test windows:
   still     +_._ points
   ...
   step size on this test set is 1/n per class - a difference smaller than one window is not a difference.

LARGEST ERROR   rule: did ____, said ____ x_   model: ...

The cost side - bytes, microseconds, RAM, Flash - is not in these files. It is on the board. Sheet section 6.
```

### If it does not look like that

- **`train.py` says `REFUSED  this folder was made with --train-only`.** Re-run `windows.py`
  with `--test`.
- **`compare.py` says `REFUSED  ... not the same test windows`.** `cm_rule.json` and
  `cm_model.json` came from different `w10_out` folders. Re-run `baseline.py` and `train.py`
  on the same folder.
- **`compare.py` says `REFUSED  the manual matrix counts __ windows but the test set has __`.**
  You did not upload exactly the `testing/` folder as the Testing category, or you copied
  percentages instead of counts.
- **The Studio rejects a file, or the labels look like `shake.sessionA`.** See
  `EDGE_IMPULSE.md`. If the gyroscope unit string is refused, re-run `windows.py` with
  `--ei-accel-only` and say so on section 4.
- **The upload is slow or the service is down.** Stop waiting. The offline path (step 1) is
  the one cycle 3 uses. Tick "Studio unavailable" on section 4 and move on.
- **The model's TEST matrix is worse than the rule's.** Possible, and a result. Section 5 is
  about the size of the difference against the step size, not about who wins.

**→ Record sheet:** section 4 (both tables, as applicable), section 5, section 6 "model bytes".

---

## Run 4 · Both classifiers on the board

**Time: about 8 minutes.**

### Do this

1. Copy `sketches/sketch_c_classify.cpp` to `src/main.cpp`. Copy the **new**
   `sketches/model_weights.h` (the one `train.py` just wrote) over the placeholder in `src/`.
2. In `src/main.cpp`, fill `T_TAP_G`, `T_SHAKE_G`, `T_TILT_DPS` with **the same three numbers**
   as section 2.
3. Build. **RAM should equal the sketch C figure in section 6 to the byte.** Write RAM and
   Flash down.
4. Upload, monitor at `115200`, RST. Read the header: `# model_ready=1 model_bytes=___`.
5. Twenty windows. Before each `>> GO`, write the class you are about to do in section 7.
   After each `>> STOP`, copy the rule's and the model's verdicts from the `L,` line.
6. After `# done live_windows=20`, close the monitor, rename the log `w10/live.txt`. Copy
   `feat_us`, `rule_us`, `mlp_us` from three windows into section 6.

### You should see

```
# who_am_i=0x__ expected=0x__ addr=0x__
# W10 classify - rule and model, live
# source=mpu6050 addr=0x__
# fs_hz=100 window_samples=200 n_feat=8
# classes=still,tilt,shake,tap
# accel_cfg=0x__ xl_sens_mg=_.____ gyro_cfg=0x__ g_sens_mdps=__.____
# model_ready=1 model_bytes=___ n_feat=8 n_hid=12 n_cls=4
# model_provenance=train=mpu6050:sessionA.txt n=__ test=sessionB.txt n=__ epochs=400 seed=0 written=2026-11-12
# rule T_TAP_G=_.___ T_SHAKE_G=_.___ T_TILT_DPS=__._
# columns: L,n,rule,model,p_model,feat_us,rule_us,mlp_us,missed,clipped,f0..f7

>> live window 1 of 20 : do ONE class, and write down which
>> 3
>> 2
>> 1
>> GO
>> STOP
L,1,____,____,_.__,___,__,___,0,0,_.___,_.___,...
```

and after twenty:

```
# done live_windows=20
```

### If it does not look like that

- **`halt=threshold_not_filled`.** One of the three constants is still `0.0f`.
- **`# model_ready=0`** and `none` in the model column. The placeholder header is still in
  `src/`. Copy the new `model_weights.h` over it and rebuild.
- **`model_bytes=` is not the number `train.py` printed.** You are running a header from a
  different pack version. Use this pack's `train.py`.
- **`mlp_us` is thousands of microseconds, not tens or hundreds.** Something is printing
  inside the timed region — did you edit the sketch? The three `micros()` pairs wrap the
  computation only; slide 39.
- **`missed` or `clipped` is not 0 on a live window.** Exclude that window from section 7's
  counts and say so in the last column.
- **RAM differs from section 6's sketch C figure.** That should not happen; tell a TA and
  write both numbers down. Flash differing by tens of bytes (the provenance string) is expected.

**→ Record sheet:** section 6 (all of it), section 7, then sections 8–10.

---

## Before you leave

The checklist at the end of `RECORD_SHEET.md`, and the Google Form. HW5 is built on every
section of the sheet; the files must be in your repo the same day, and **you cannot re-create
a session once the board is in a different hand on a different afternoon.**
