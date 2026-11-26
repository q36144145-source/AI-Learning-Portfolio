# PREFLIGHT — required before Thursday 19 Nov

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


**Fifteen minutes. Needs internet and needs your board.** Do it at home, on the laptop
you are bringing to class.

Nothing new has to be installed. The two scripts are standard library only. So why a
preflight?

**Two reasons, and the second is new this week.**

1. The first build of a fresh project compiles the whole Arduino core and takes two to
   three minutes; every build after that takes a few seconds. Pay the slow one tonight.
2. **This week's `platformio.ini` has two extra lines that switch the build to the
   board's octal PSRAM.** If they are missing or wrong, nothing fails: the firmware boots,
   the sketch runs, and every "PSRAM" figure you measure on Thursday is silently an SRAM
   figure. The sketch catches this and refuses the PSRAM tests — but you want to find
   that out tonight, not at 09:40 with forty-four other people.

**Come to class with all four boxes ticked.**

- [ ] 1 · A `w11` project exists and builds Sketch 1
- [ ] 2 · The board uploads, the monitor catches the `=== W11 REPORT ===` block
- [ ] 3 · **`psram_bytes` in that block is not 0**
- [ ] 4 · `report.py` runs on the log and ends with `VERDICT  valid`

---

## 1 · Build Sketch 1 once, tonight

Make a **new** PlatformIO project (do not reuse an old one — the `platformio.ini` is
different this week), board `esp32-s3-devkitc-1`, framework Arduino. Then:

1. Copy this pack's `platformio.ini` over the generated one. **Open it and read the two
   lines under "the two lines that matter this week".**
2. Copy **`sketches/w11_common.h`** into `src/`.
3. Copy **`sketches/w11_tables.cpp`** into `src/`. (Sketch 1 does not use it; it is
   there so that you never have to remember it on Thursday.)
4. Copy **`sketches/sketch_1_hierarchy.cpp`** into `src/` and rename it `main.cpp`.

Press Build. You want `SUCCESS` and the two familiar lines:

```
RAM:   [=         ]   5.7% (used ..... bytes from 327680 bytes)
Flash: [=         ]   8.2% (used ...... bytes from 3342336 bytes)
```

**Write those two byte counts down and bring them.** You will compare them against
sketches 2 and 3 in class, and against what the linker printed on my machine.

> Three files go in `src/`. `w11_common.h` is included as `"w11_common.h"` with quotes,
> so it has to sit next to `main.cpp`. `w11_tables.cpp` has to be in `src/` too, or
> sketch 2 fails to link on Thursday with `undefined reference to W11_W_F32_BITS`.

## 2 · Upload and watch it finish

Upload, open the Serial Monitor at `115200`, and press RST. The sketch waits 3 seconds,
runs for a few more, prints a block that starts `=== W11 REPORT ===` and ends
`=== END ===`, and then stops.

- [ ] **Did you see the `=== W11 REPORT ===` line?** If the monitor attached too late and
      you only caught the tail, press RST again with the monitor open.
- [ ] **Did it stop?** It should print once and go quiet. If it loops, reboots, or prints
      a stack trace, bring the whole text.

## 3 · The one line to read tonight

In the block, find:

```
psram_found_api      = _
psram_bytes          = _______
```

**`psram_bytes` must not be 0.** On our N16R8 boards it is a little under 8,388,608.

If it **is** 0, your build cannot see the PSRAM. Check, in this order:

1. `platformio.ini` in the project is the pack's, with
   `board_build.arduino.memory_type = qio_opi` and `build_flags = -DBOARD_HAS_PSRAM`.
2. You did a **Clean** after changing `platformio.ini` (PlatformIO → Clean, then Build).
   A stale build keeps the old libraries.
3. The board really is an N16R8 — the module label on the metal can says
   `ESP32-S3-WROOM-1-N16R8`.

If `psram_bytes` is still 0 after those three, post in the channel **tonight** with the
whole report block. Do not wait for Thursday: with 0 bytes of PSRAM, two of the three
runs lose half their rows.

**Also read `psram_found_api` next to it.** If it says `1` while `psram_bytes` says `0`,
you have just seen an API return true without checking. The record sheet asks about this.

**Do not write the ns figures from tonight's run into your record sheet.** Tonight's
run is a smoke test. The measurements that count are the ones you take in the room, in
order, with the record sheet in front of you.

## 4 · `report.py` reads it

Close the monitor. With `monitor_filters = log2file` a
`logs/device-monitor-YYMMDD-HHMMSS.log` appears in the project folder.

Copy the newest file from your project's `logs/` folder into the **pack folder** (where
`scripts/` is — the script is not in your PlatformIO project, and the log is not in the
pack). Then, from the pack folder:

```bash
python scripts/report.py "device-monitor-*.log"
```

It should list the figures, say `ok` or `REFUSED` for each, and end with
`VERDICT  valid`. If it says `NOT VALID`, read the reason: *no block* means step 2 did not
really succeed; *matches 2 files* means more than one log is in the folder — name the one
you mean; *psram ... REFUSED* with `psram_bytes = 0` is step 3.

`python --version` needs to be **3.8 or newer**, settled in W03.

---

## If you get stuck

Post in the course channel with **your OS, the command you ran, and the complete error
message** — or the complete report block. Somebody else has the same problem and will be
glad you asked.
