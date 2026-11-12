# PREFLIGHT — required before Thursday 12 Nov

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


**About twenty-five minutes. Needs internet, your board, and your IMU on its four wires.**
Do it at home, on the laptop you are bringing to class.

Why this one is longer than W07's: this week has **two** things the room's network cannot be
trusted with — the first build of a fresh project, and an account on a web service. Both are
done tonight. The session itself needs the network for one upload, and has a fallback if even
that fails.

**Come to class with all five boxes ticked.**

- [ ] 1 · A `w10` project exists and builds Sketch A
- [ ] 2 · Your five W05 datasheet values are in `w10_imu.h`, and the board gets past the bring-up
- [ ] 3 · One throw-away session ran to `# done windows=32`
- [ ] 4 · You have an Edge Impulse account and one empty project
- [ ] 5 · `python scripts/selftest.py` says `19 of 19 checks passed`

---

## 1 · Build Sketch A once, tonight

Make a **new** PlatformIO project (do not reuse W07's), board `esp32-s3-devkitc-1`, framework
Arduino. Then:

1. Copy this pack's `platformio.ini` over the generated one.
2. Copy **all three** of `sketches/w10_imu.h`, `sketches/model_weights.h` and
   `sketches/sketch_a_collect.cpp` into `src/`, renaming the `.cpp` to `main.cpp`.

Build. You want `SUCCESS` and the two familiar lines; the denominators are `327680` and
`3342336`. **Write both byte counts down.** RAM should match the figure in `README.md` to the
byte; Flash should match it too. **If either does not, that is worth a sentence in your report** —
it means something about your toolchain or your source differs from ours.

> `w10_imu.h` is included with quotes, so it has to sit next to `main.cpp`. The classify
> sketch also needs `model_weights.h` next to it. If the build says `No such file or
> directory`, that is the whole problem.

## 2 · Fill the five blanks — from your W05 record sheet

Open `src/w10_imu.h` and find the block of five `volatile const` lines near the top. Four of
them are the values you found in the datasheet in W05 (`WHOAMI_EXPECT`, `ACCEL_CFG_VALUE`,
`GYRO_CFG_VALUE`, `XL_SENS_MG`); copy them in. The fifth, `G_SENS_MDPS`, is the gyroscope's
sensitivity at the range you set, in **milli-degrees per second per LSB** — same table in
the datasheet as the fourth, one row over.

Until all five are filled, the sketch prints `halt=blank_not_filled` and stops. That is the
same refusal as W05's sketch 4, for the same reason: a window logged under an unknown scale
is a column of integers, not a measurement.

Upload, open the Serial Monitor at `115200`, press RST. The first lines should be the
`# who_am_i=` line and then the `# W10 collect` header. If you see any `halt=` line, read it:
it names the step that failed, and W05's lab guide covers every one of them.

## 3 · One throw-away session

Let it run. It will prompt you thirty-two times — `still`, `tilt`, `shake`, `tap` in a fixed
mixed order — with a 3-2-1 countdown each. Do roughly what it says. About four minutes.

**What has to move is the GY-521, not the DevKit.** Hold the breadboard the module is seated in (with the DevKit riding along if it is plugged into the same breadboard). If the DevKit sits on the desk on its own, that is fine — the four jumpers follow. Holding the DevKit while the breadboard stays flat records `still` for all four classes, and nothing will tell you until the feature table shows four identical columns.

- [ ] Did it reach `# done windows=32 ...`?
- [ ] Did a `logs/device-monitor-YYMMDD-HHMMSS.log` appear when you closed the monitor?
- [ ] Open the log. Does every `W n label end` line say `missed=0`? If some say `missed=1`
      or more, tell us in the channel with your laptop model — it means the serial dump of
      the previous window ran into the next tick, and the pacing needs looking at before
      Thursday, not on Thursday.

**Then delete that log, or name it `preflight-NOT-DATA.txt`.** Thursday's two sessions are
recorded in the room, in order, with the record sheet in front of you. Tonight's run is a
smoke test on a board you are still learning to hold, and it must not become session A.

## 4 · Edge Impulse account — five minutes, no card

1. <https://studio.edgeimpulse.com> → create a free account (the Developer Plan). Use an
   address you are content to have on a service owned by Qualcomm; `EDGE_IMPULSE.md` says
   what the plan includes and why that sentence is there.
2. Create one project named `bme3301-w10`. Leave it empty.
3. Log out, log in again. Thursday is not the day to discover a password problem.

If you decide not to make an account — that is allowed — write "no account, offline path" on
your record sheet now. You lose nothing on Thursday except the comparison with the Studio's
confusion matrix; the model you deploy is built offline either way.

## 5 · The scripts run

`python --version` needs 3.8 or newer and **numpy** installed — both settled in W03. Then,
from the pack folder:

```bash
python scripts/selftest.py
```

It generates two **synthetic** sessions, runs every script on them, and checks that each one
**refuses** what it should: a synthetic file without `--selftest`, a random split, the same
file as train and test, a short class, a zero threshold, mismatched test windows — and that a
matrix scored on its own synthetic data is marked `NOTE   source=SYNTHETIC`. It ends with
`19 of 19 checks passed`. It needs no board.

It leaves a `selftest_out/` folder behind. Everything in it is synthetic and says so; delete
it or leave it, but do not hand anything from it in.

## If you get stuck

Post in the course channel with **your OS, the command you ran, and the complete error
message** — or, for the board, the complete `halt=` line. Somebody else has the same problem.
