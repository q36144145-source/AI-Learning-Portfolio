# W11 Lab Guide — run by run, at your own pace

**Embedded Systems (BME 3301) · Week 11 · Thursday 19 Nov, 09:10–12:00 · 醫技醫工館 5776**

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


This is the document you drive yourself. The slides move at one speed for forty-five people;
this does not.

Every run has the same three parts:

| | |
|---|---|
| **Do this** | Numbered steps. One action each. Do them in order. |
| **You should see** | The *shape* of the output — the keys, the units, the block markers. **Never a value.** Every number today is yours. |
| **If it does not look like that** | The ways this step actually goes wrong, and what to do about each. Read this column **before** you raise your hand. |

Each run ends with **→ Record sheet**.

> **Three runs, one question.** Run 1 times the three memories with no model in the way.
> Run 2 puts one dense layer's weights in each of them, in float32 and int8. Run 3 reproduces
> the access pattern that lets a 25-million-parameter table sit in flash. The record sheet's
> last ratio — a dense inference against a flash lookup — is the week's argument in one number.

> **Every figure is a median of five passes, and the sketch refuses figures it cannot stand
> behind.** A `REFUSED` line is not a failure of yours. It is the sketch telling you *which*
> number not to trust and *why*. Copy the reason, not a guess.

---

## Before you start — how the screenshots work

Reference frames belong in this guide, and the deck has six. **None have been captured
yet** — the TA dry run on Tue 17 Nov produces them, and the file names here are the deck's,
so one capture serves both.

> **📷 SCREENSHOT — NOT CAPTURED YET**
> `example-name.png` — one line saying what the frame shows.

---

## Run 0 · Set the project up

**Time: about 6 minutes.** Most of it was done in PREFLIGHT.

### Do this

1. Unzip `W11_student_pack.zip` **next to** the earlier packs, not into them.
2. Open your `w11` PlatformIO project from PREFLIGHT. Confirm `platformio.ini` is the pack's
   — it has **eight** lines that matter, not six: look for `memory_type = qio_opi` and
   `-DBOARD_HAS_PSRAM`.
3. Make a folder `w11/` — **inside the unzipped pack folder, next to `scripts/`, not inside
   your PlatformIO project** (which is also called `w11`, so this is easy to get wrong).
   Everything you produce today goes in it, and every `python scripts/...` command today runs
   **from the pack folder**. Each log the monitor writes into the project's `logs/` gets renamed
   and moved into this `w11/`. At the end, copy the whole `w11/` folder into your repo. If a script
   says a file does not exist, check which of the two `w11` folders you are standing in.
4. `src/` holds three files for the whole session: `w11_common.h`, `w11_tables.cpp`, and
   `main.cpp`. Only `main.cpp` changes between runs.
5. Nothing is wired. Sketch 1 drives GPIO 2 once as a demonstration and **refuses** GPIO 35,
   which you can read about in the report.

### You should see

A build that succeeds with the two familiar denominators, `327680` and `3342336`.

### If it does not look like that

- **`fatal error: w11_common.h: No such file`.** The header has to be in `src/` next to
  `main.cpp`. Copy it once and leave it there.
- **`undefined reference to 'W11_W_F32_BITS'`.** `w11_tables.cpp` is not in `src/`. Sketch 2
  needs it; put it there now so you do not have to remember it later.
- **The build header says `No PSRAM` / `8 MB QD, No PSRAM`.** Ignore that line. It is the
  board definition's name, printed before our two `platformio.ini` lines take effect. The only
  test that counts is `psram_bytes` in the report block — just under 8,388,608 means it works.
- **The build is on the old `platformio.ini`.** Copy the pack's over it, then **Clean** before
  you build. A stale build keeps the quad-PSRAM libraries and `psram_bytes` comes out 0.

**→ Record sheet:** your name, the date. **Then section 1 — the prediction — before run 1.**

---

## Run 1 · The same bytes, three memories

**Time: about 10 minutes, most of it build and upload. The board runs for about five seconds.**

### Do this

1. Fill in record sheet **section 1** now. Three predictions, no numbers needed.
2. Copy `sketches/sketch_1_hierarchy.cpp` into `src/main.cpp`.
3. Build. Write the `RAM:` and `Flash:` byte counts into **section 6**.
4. Upload, open the monitor at `115200`, **then press RST**.
5. Wait for `=== END ===`. About eight seconds from reset.
6. Stop the monitor. Rename the newest log `run1-hierarchy.txt`, move it into `w11/`.
7. ```bash
   python scripts/report.py w11/run1-hierarchy.txt
   python scripts/ratios.py w11/run1-hierarchy.txt
   ```

### You should see

A report block, opened and closed by markers so a script can find it in any log. The top
is the same for every sketch; then each tier prints its address, its tier as the sketch
classified it, a balance line, and two tests:

```
=== W11 REPORT ===
sketch               = 1_hierarchy
cpu_mhz              = ___
repeats              = 5
spread_max_pct       = 25
min_cycles           = 20000
dcache_bytes         = 32768
line_bytes           = 32
psram_found_api      = _
psram_bytes          = _______
internal_largest     = ______
words_128k           = 32768
words_1m             = 262144
pin_refused          = GPIO35: wired to the octal PSRAM on this module (psram_bytes=_______), not driven
pin_2                = driven
malloc_2k_tier       = ____
malloc_64k_tier      = _____
sram_words = _____
sram_addr = 0x________
sram_tier = SRAM
sram_balance = ok
sram_seq_status = ok
sram_seq_ns_per_item = ____
sram_seq_first_ns_per_item = ____
sram_seq_spread_pct = ___
sram_seq_checksum = 0x________
sram_rand_status = ok
sram_rand_ns_per_item = ____
...
psram128k_words = 32768
...
flash1m_rand_checksum = 0x________
control_noload_status = ok
control_noload_ns_per_item = ____
...
=== END ===
```

Eleven tests in all: `seq` and `rand` for `sram`, `psram128k`, `psram1m`, `flash128k`,
`flash1m`, plus `control_noload`. Each has a `_status` of `ok` or `REFUSED: <reason>`.

**Read `psram_bytes` first.** If it is 0, the four `psram*` tests say `REFUSED: PSRAM heap is
0 bytes: ...` and the rest of the block is still valid. That is a correct report of a build
problem, and it is worth fixing **now**, before run 2, where it costs you two of six cells.

`report.py` lists every figure with `ok` or `REFUSED`, repeats the sketch's own checks from
the printed numbers, and ends `VERDICT valid` or `VERDICT NOT VALID`. `ratios.py` prints the
seq/rand table and the ratios section 2 asks for; a ratio with a refused side is `n/a`.

> **📷 SCREENSHOT — NOT CAPTURED YET**
> `serial-01-report-hierarchy.png` — the top of the block down to the end of the `sram` tests.

> **📷 SCREENSHOT — NOT CAPTURED YET**
> `stats-01-ratios-run1.png` — `ratios.py`'s RUN 1 table and the ratios under it.

### If it does not look like that

- **`psram_bytes = 0` and `psram_found_api = 1`.** The API said yes without checking; the
  heap says no. Fix `platformio.ini` (two lines), **Clean**, build, upload, run again. Section
  0 of the sheet asks you to explain the disagreement — you have just seen it live.
- **`sram_words = 16384`, not 32768.** The heap had no free 128 KB block, so the sketch used
  64 KB and said so. The SRAM figures are still valid — SRAM has no cache, so the size does
  not change the per-word cost — but write the size you got.
- **A `rand` test says `REFUSED: unstable across passes`.** Something interrupted a pass.
  Press RST and run again with nothing else talking to the board. If it repeats, the figure
  stays refused and you write that.
- **A test says `REFUSED: faster than the hardware floor`.** The compiler removed the loop.
  That should be impossible with the pack's kernels; if you see it, you edited the sketch.
  Put the pack's back.
- **`_balance = MISMATCH`.** The sequential and random passes did not read the same words.
  Both tests are refused. Take the run again; if it repeats, bring the block to a TA — that is
  a real finding.
- **`=== END ===` never arrives.** Press RST with the monitor already open. If the board
  reboots mid-block, bring the whole text including the reset reason.
- **Two blocks in one file.** You reset twice. One run per file, or `--block 2`.

**→ Record sheet:** sections **0** and **2**.

---

## Run 2 · One dense layer, six ways

**Time: about 12 minutes. Build is longer — `w11_tables.cpp` is 600 KB of text — and the
board runs for a few seconds.**

### Do this

1. Copy `sketches/sketch_2_dense.cpp` into `src/main.cpp`. `w11_tables.cpp` stays.
2. Build. Write the `RAM:` and `Flash:` byte counts into **section 6**. Notice the Flash figure.
3. Upload, monitor at `115200`, RST, wait for `=== END ===`.
4. Rename the newest log `run2-dense.txt`, move it into `w11/`.
5. ```bash
   python scripts/report.py w11/run2-dense.txt
   python scripts/ratios.py w11/run1-hierarchy.txt w11/run2-dense.txt
   ```

### You should see

```
=== W11 REPORT ===
sketch               = 2_dense
...
n_in                 = 256
n_out                = 128
macs_per_inference   = 32768
weights_bytes_f32    = 131072
weights_bytes_s8     = 32768
scale_bytes_s8       = 4
scale_w              = _.________e-__
scale_x              = _.________e-__
table_check          = ok
f32_flash_addr = 0x3C______
f32_flash_tier = FLASH
f32_flash_status = ok
f32_flash_ns_per_item = ____
f32_flash_first_ns_per_item = ____
f32_flash_spread_pct = ___
f32_flash_checksum = 0x________
f32_flash_us_per_inference = ____
s8_flash_addr = 0x3C______
...
f32_sram_... s8_sram_... f32_psram_... s8_psram_...
f32_outputs_identical = ok
s8_outputs_identical  = ok
quant_rms_err         = _.______
quant_rms_ref         = _.______
quant_rel_rms_err_pct = _.____
quant_max_abs_err     = _.______
quant_half_step_w     = _.______e-__
=== END ===
```

Six tests: `f32_` and `s8_` × `sram`, `psram`, `flash`. The `_ns_per_item` is ns per
multiply-accumulate; `_us_per_inference` is the same number times 32,768, which `report.py`
recomputes and refuses the block if it disagrees.

**`table_check` must be `ok`.** It says the bytes the linker put in flash are the bytes
`gen_tables.py` wrote. If it is `MISMATCH`, every test is refused and the block says why.

**`f32_outputs_identical` and `s8_outputs_identical` must be `ok`.** The same arithmetic on
the same bytes in three memories gave the same 128 numbers. If it did not, something else was
running on the board, and the block is not valid.

> **📷 SCREENSHOT — NOT CAPTURED YET**
> `serial-02-report-dense.png` — from `table_check` through the six tests to the quant lines.

### If it does not look like that

- **`undefined reference to 'W11_W_F32_BITS'` at link.** `w11_tables.cpp` is not in `src/`.
- **`table_check = MISMATCH`.** The `w11_tables.cpp` in `src/` is not the pack's — edited,
  truncated by a copy, or from a different seed. Copy the pack's again, or regenerate it with
  `python scripts/gen_tables.py` and copy the result. The block's `table_check_detail` line
  shows both sums.
- **`f32_psram` and `s8_psram` refused with `PSRAM heap is 0 bytes`.** Same fix as run 1. You
  still have four valid cells; take run 2 again after the fix if there is time, otherwise
  write the two as refused and say why.
- **`f32_sram` refused with `no internal block of 128 KB free`.** The heap could not give the
  float weights a contiguous 128 KB. Write it as refused; the int8 SRAM cell (32 KB) should
  still be there.
- **`quant_rel_rms_err_pct = n/a (reference output is zero)`.** Cannot happen with the pack's
  tables; if you see it, `table_check` was not `ok` either.
- **The Flash figure at build jumped by about 164 KB compared with run 1.** Correct. Section 6
  asks why.

**→ Record sheet:** section **3**, and the two build figures in section **6**.

---

## Run 3 · Six rows per token

**Time: about 8 minutes.**

### Do this

1. Copy `sketches/sketch_3_sparse.cpp` into `src/main.cpp`.
2. Build; write the `RAM:` and `Flash:` figures into **section 6**.
3. Upload, monitor, RST, wait for `=== END ===`.
4. Rename the newest log `run3-sparse.txt`, move it into `w11/`.
5. ```bash
   python scripts/report.py w11/run3-sparse.txt
   python scripts/ratios.py w11/run1-hierarchy.txt w11/run2-dense.txt w11/run3-sparse.txt
   ```

### You should see

```
=== W11 REPORT ===
sketch               = 3_sparse
...
rows_per_token       = 6
row_bytes            = 64
bytes_per_token      = 384
tokens_per_pass      = 256
sram_holds_table     = no (largest internal block ______, table 1048576)
flash1m_lookup_rows = 16384
flash1m_lookup_table_bytes = 1048576
flash1m_lookup_addr = 0x3C______
flash1m_lookup_tier = FLASH
flash1m_lookup_status = ok
flash1m_lookup_ns_per_item = _____
flash1m_lookup_first_ns_per_item = _____
flash1m_lookup_spread_pct = ___
flash1m_lookup_checksum = 0x________
flash1m_lookup_us_per_token = ____
flash16k_lookup_rows = 256
...
psram1m_lookup_... psram16k_lookup_... sram16k_lookup_...
=== END ===
```

Five tests. **Two of them are the measurement** — `flash1m_lookup` and `psram1m_lookup`, a
1 MB table. **Three of them are the trap, printed on purpose** — the `16k` tests run the
identical code against a 16 KB table that fits inside the 32 KB data cache. After the first
pass every row is a cache hit, and the figure says nothing about flash or PSRAM. Compare each
`16k` test's `_first_ns_per_item` with its median: the first pass is the only cold one.

With all three files, `ratios.py` adds the last line of the sheet: one int8 inference from
SRAM (run 2) against one six-row lookup from flash (run 3).

> **📷 SCREENSHOT — NOT CAPTURED YET**
> `serial-03-report-sparse.png` — the whole block.

> **📷 SCREENSHOT — NOT CAPTURED YET**
> `stats-02-ratios-all.png` — `ratios.py` with all three files, RUN 3 section and the ACROSS RUNS line.

### If it does not look like that

- **`sram_holds_table = yes`.** The heap reported a free internal block of 1 MB or more. That
  cannot happen on a 512 KB SRAM; bring the block to a TA.
- **`psram*` refused.** Same two lines in `platformio.ini`. The flash figures stand on their own.
- **`flash1m_lookup` and `flash16k_lookup` are nearly the same.** Read both `_first_ns_per_item`
  lines. If the 1 MB figure is close to the 16 KB one, the scramble is landing on few rows
  — it should not with the pack's kernel. Bring the block.
- **ACROSS RUNS says `n/a`.** One of `s8_sram` (run 2) or `flash1m_lookup` (run 3) was refused.
  The ratio is not formed from a refused side; fix that run or write `n/a` with the reason.

**→ Record sheet:** section **4**. Then **`EXERCISE_placement.md`** — eight minutes, handed in
before the reveal.

---

# Before you leave

- [ ] `run1-hierarchy.txt`, `run2-dense.txt`, `run3-sparse.txt` in `w11/` — each with a
      complete `=== W11 REPORT ===` block that `report.py` calls `valid`
- [ ] Record sheet sections **0 to 6**; section **1** written **before** run 1
- [ ] Sections **7**, **8**, **9** — one sentence each, all three required
- [ ] Every refused figure written as `refused: <reason>`
- [ ] `EXERCISE_placement.md` handed in
- [ ] Two `AI_USAGE.md` entries

**Section 9 is the one people skip.** Everything today was timed by the board under test,
with its own cycle counter, and the first pass of every test ran against a cache in a state
nobody controlled. Say what that means for the numbers you are handing in.
