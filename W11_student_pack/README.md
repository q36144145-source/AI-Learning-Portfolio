# W11 Student Pack — TinyML II and the Memory Hierarchy

**Embedded Systems (BME 3301) · Week 11 · Thursday 19 Nov · online this week**

> **No class in the room on 19 Nov.** Lecture and lab videos are on Moodle. Everything below
> still applies; **"today" means "by Wed 25 Nov, 23:59"**, and the placement exercise is
> submitted on Moodle *before* you watch the answer slide.

Everything you run in class today is in this folder. You do not have to type any of it
from the slides — the files here are character-for-character identical to what is
projected.

**`PREFLIGHT.md` is required before you come to class.** About fifteen minutes, needs
the board. It builds one sketch so the slow first compile happens at home, and — this
week — it checks one line of output that tells you whether your build can see the
board's PSRAM at all. A board whose PSRAM is not enabled measures the wrong thing on
Thursday, and nothing will warn you.

---

## What today is

In W02 you were shown a pyramid: internal SRAM at the top, external PSRAM in the middle,
flash at the bottom, each a step slower and a step bigger. You were told the step sizes
would be measured in W11. This is W11.

You will read the same bytes from all three memories and time it. Then you will run one
dense neural-network layer with its weights placed in each memory, in float32 and in
int8, and time that. Then you will reproduce the access pattern that lets a real
28.9-million-parameter language model run on exactly this board, and time that too.

The numbers you leave with are **nanoseconds per word, microseconds per inference and
microseconds per token** — for your board, measured by you. They are not on any slide.

## What is in here

```
platformio.ini              W03's six lines PLUS two that enable the octal PSRAM. Read them
sketches/
  w11_common.h              the clock, the tier test, the report format, the REFUSAL rules.
                            Included by all three sketches so the only difference between
                            runs is what is being timed
  w11_tables.cpp            GENERATED (scripts/gen_tables.py): one 256x128 dense layer as
                            const data, so the linker puts it in flash. 600 KB of text,
                            160 KB of bytes. Needed by sketch 2 only, harmless for 1 and 3
  sketch_1_hierarchy.cpp    sequential and random reads: SRAM, PSRAM, flash    (cycle 1)
  sketch_2_dense.cpp        y = W x, float32 and int8, weights in each tier    (cycle 2)
  sketch_3_sparse.cpp       6 rows per token out of a 1 MB table               (cycle 3)
scripts/
  gen_tables.py             regenerates w11_tables.cpp (same seed, same bytes)
  report.py                 pulls one report out of a log, re-checks it, and lists per
                            figure what may be copied and what the sketch refused
  ratios.py                 the ratios the record sheet asks for, from valid runs only
reading/
  esp_nn_fully_connected_ansi.c          the scalar reference kernel       (cycle 3, L4)
  esp_nn_fully_connected_s8_esp32s3.S    the SIMD version, read not written (cycle 3, L4)
  SOURCES.md                where those two files came from, commit and date
LAB_GUIDE.md                run by run, at your own pace. Work from this in class
RECORD_SHEET.md             fill this in as you go. It is the deliverable
EXERCISE_placement.md       the in-class exercise: three components, three tiers, your
                            reasons. Hand it in before the reveal
AI_USAGE_template.md        the two entries you owe in AI_USAGE.md
PREFLIGHT.md                do this before class
```

`report.py` and `ratios.py` are **standard library only**. Nothing to `pip install`.

## The two lines in `platformio.ini` that decide whether today works

```
board_build.arduino.memory_type = qio_opi
build_flags = -DBOARD_HAS_PSRAM
```

The board definition this platform version ships under the name `esp32-s3-devkitc-1` is
the **N8**: 8 MB flash, **no PSRAM**. Ours is the **N16R8**: 16 MB flash, **8 MB octal
PSRAM**. Without those two lines the build links libraries for a quad-SPI PSRAM, the
octal chip does not answer, the firmware carries on anyway, and every "PSRAM" buffer
silently lands in internal SRAM. You would measure SRAM twice and call one of them PSRAM.

Every sketch therefore prints `psram_bytes` from the heap and refuses its PSRAM tests when
it is zero. It does **not** trust `psramFound()`, and the record sheet asks you why.

## The runs

Three builds, three runs, all required. Rename each log as you finish it and put it in
your repo under `w11/`:

| Run | Sketch | Cycle | Save log as |
|---|---|---|---|
| 1 | `sketch_1_hierarchy.cpp` | 1 | `run1-hierarchy.txt` |
| 2 | `sketch_2_dense.cpp` | 2 | `run2-dense.txt` |
| 3 | `sketch_3_sparse.cpp` | 3 | `run3-sparse.txt` |

Each run prints one `=== W11 REPORT ===` block a few seconds after reset and then stops.
`w11_common.h` and `w11_tables.cpp` sit in `src/` for all three; only `main.cpp` changes.

## What you produce today

| File | From |
|---|---|
| `run1-hierarchy.txt`, `run2-dense.txt`, `run3-sparse.txt` | the three runs |
| `RECORD_SHEET.md` | filled in |
| `EXERCISE_placement.md` | filled in, **handed in before the reveal** |
| `w11/AI_USAGE.md` entries | two of them |

There is no HW this week. **These numbers are the first real data in HW6's power budget**
(W12), so they go in the repo the same day, like every other week.

## How a figure is refused

Every timed test prints a `<name>_status` line. It is either `ok`, and then
`<name>_ns_per_item` is a number you may copy, or `REFUSED: <reason>`, and then it is
`n/a`. The sketch refuses when:

| Reason printed | What happened |
|---|---|
| `buffer is in X, not Y` | the pointer was not in the memory the test claims. The address is printed next to it |
| `PSRAM heap is 0 bytes: ...` | the build cannot see the PSRAM; the two `platformio.ini` lines above |
| `faster than the hardware floor: the compiler removed the work` | the figure is below one CPU cycle per item, which no load can do |
| `pass too short to time` | the total was within a few resolution steps of the clock |
| `unstable across passes` | (max − min) / median over the five passes exceeded 25 % |
| `passes read different data` / `balance = MISMATCH` | the checksums of passes that read the same bytes disagree |

`report.py` repeats every one of those checks from the printed numbers, and adds the one
the board cannot do: it recomputes the sketch's arithmetic and refuses a block that
disagrees with itself. **Only a figure marked `ok` in a block whose verdict is `valid`
goes on the sheet.** A refused figure is a result too — write "refused: reason", not a
guess.

## Units and conventions

- **ns per word** (run 1): a word is 4 bytes. Multiply by 8 for one 32-byte cache line.
- **µs per inference** (run 2): one pass of 32,768 multiply-accumulates. Also printed as
  ns per MAC.
- **µs per token** (run 3): six 64-byte rows read out of a 1 MB table.
- Every figure is the **median of five passes**; the first pass is printed separately
  because its cache state is unknown.
- `cpu_mhz` is printed. All ns figures are derived from the cycle counter at that rate.

A number without a unit is not a measurement, and a ratio whose two sides were not
measured the same way is not a ratio.

## The pin header

Three header pins — **GPIO 35, 36 and 37** — are the octal PSRAM's own lines on this
module. `pinMode(35, OUTPUT)` compiles and then corrupts the memory you are about to
benchmark. `w11_pinModeChecked()` refuses them and prints why; sketch 1 calls it once so
you can see the refusal. Nothing is wired today. The silkscreen number is the GPIO number.
