# W11 Record Sheet

> **Correction, 11 Nov 2026 — replaces the 14 Oct note.** **RAM reproduces exactly; Flash does not, and the reason is now measured.** The Arduino framework compiles the full path of its own source files into the binary, and that path runs through your user folder (`C:\Users\<name>\.platformio\…`). So Flash changes with the **length of your Windows username** — not with your project folder, which the 14 Oct note tested correctly but generalised too far from. Measured 11 Nov 2026: the same sketch built through a PlatformIO path 28 characters shorter came out 128 bytes smaller in Flash, with RAM unchanged. **A RAM difference is a signal; a Flash difference of tens of bytes is your username.**

**Name** 陳竑勳  **Student ID** Q36144145  **Date** 2026-11-26（線上週；在 25 Nov 期限之後才完成）

Every cell below is measured by you, in this room, today. **There is no reference answer
on any slide in this deck, and none in this folder.** If a cell is empty at 12:00, say
so — an empty cell is a result, a guessed cell is not. If the sketch **refused** a figure,
write `refused:` and the reason it printed. That is a result too.

The only pre-filled numbers anywhere in this pack are the Flash and RAM byte counts in
section 6, because the linker — not the board — produces them.

---

## 0 · Before anything: the build can see the PSRAM

From run 1's block:

| | Value |
|---|---|
| `psram_found_api` | 1 |
| `psram_bytes` | 8,388,607（接近 8,388,608，PSRAM 確實有被 heap 拿到） |
| `internal_largest` (bytes) | 335,860 |
| `malloc_2k_tier` | SRAM |
| `malloc_64k_tier` | PSRAM |
| `pin_refused` (copy the line) | `GPIO35: wired to the octal PSRAM on this module (psram_bytes=8388607), not driven` |

**Two different answers to "is there PSRAM?" Which one did the sketch trust, and why
is the other one not a test?** (Read the comment above `w11_psramBytes()` in
`w11_common.h`.)

> sketch 信的是 `psram_bytes`（`heap_caps_get_total_size(MALLOC_CAP_SPIRAM)`，也就是 heap 實際拿到多少 bytes 的 PSRAM），不是 `psram_found_api`（`psramFound()`）。`w11_common.h` 的註解說明：這個 build 的 PSRAM 是 bootloader 開的（`CONFIG_SPIRAM_BOOT_INIT`），`psramInit()` 不檢查就回傳 true；就算 init 失敗，系統也會繼續跑（`CONFIG_SPIRAM_IGNORE_NOTFOUND`），所以 `psramFound()` 在沒有 PSRAM 的 build 上一樣會說 1，它不是測試。heap 的大小才是實際結果：這次兩者都說有（1 和 8,388,607），但只有後者能證明。

**A plain `malloc(65536)` landed in PSRAM. Which line of your code asked for that?**

> 沒有任何一行要求放 PSRAM。`sketch_1_hierarchy.cpp` 第 87 行只是普通的 `void *m64k = malloc(65536);`。決定放哪裡的是 framework 的設定：`qio_opi` 這組函式庫的 `sdkconfig.h` 有 `CONFIG_SPIRAM_USE_MALLOC 1` 和 `CONFIG_SPIRAM_MALLOC_ALWAYSINTERNAL 4096`，也就是 malloc 會用 PSRAM，而 4096 bytes 以下優先放內部 SRAM，超過的就可能放到 PSRAM。所以 2 KB 進 SRAM、64 KB 進 PSRAM，程式碼裡看不出來，也沒有任何警告。

---

## 1 · Before you measure: predict (slide 13)

Do this **before** run 1.

**(a)** Rank the three memories, fastest first, for a *sequential* read:
SRAM < PSRAM < Flash

**(b)** For a *random* read of the same bytes, will each tier get slower by the same
factor? Which tier will change most, and why? (Think: what does the cache do with the
seven words next to the one you asked for?)

> 不會一樣。SRAM 沒有 cache，循序和隨機幾乎一樣快；PSRAM 和 Flash 循序讀時一次 cache miss 會抓進 32 bytes（8 個 word），後面 7 個都命中，隨機讀時這 7 個用不到，每個 word 都要付一次外部讀取，所以會慢好幾倍。Flash 補 cache line 最慢，所以變得最多。

**(c)** Same tier, same random pattern, working set **1 MB instead of 128 KB**: faster,
slower, or the same? The data cache is 32 KB.

> 更慢。cache 只有 32 KB，128 KB 時大約還有 1/4 的資料能留在 cache 裡命中，1 MB 時只剩約 3%，幾乎每次都要到外部讀。
>
> （預測寫於 Run 1 之前，2026-11-26。推理過程參考了 Claude Code 的建議，我同意後採用。）

> Keep this. If the measurement disagrees, the interesting question is *why*.

---

## 2 · Run 1 · ns per word

Fill each row from `report.py`'s FIGURES list. A `REFUSED` figure is written as
`refused: <reason>`.

| Tier / working set | `_words` | `_balance` | seq ns/word | rand ns/word | rand ÷ seq |
|---|---|---|---|---|---|
| `sram` (128 KB, or the size printed) | 32768（128 KB） | ok | 29.41 | 147.05 | 5.00 |
| `psram128k` | 32768 | ok | 72.65 | 486.18 | 6.69 |
| `psram1m` | 262144 | ok | 72.50 | 571.96 | 7.89 |
| `flash128k` | 32768 | ok | 151.17 | 953.06 | 6.30 |
| `flash1m` | 262144 | ok | 151.16 | 1182.31 | 7.82 |
| `control_noload` (no memory read) | — | — | — | 130.19 | — |

```
python scripts/ratios.py w11/run1-hierarchy.txt
```

| Ratio (from `ratios.py`, `n/a` if either side was refused) | Value |
|---|---|
| psram128k seq ÷ sram seq | 2.47 |
| psram128k rand ÷ sram rand | 3.31 |
| flash128k seq ÷ sram seq | 5.14 |
| flash128k rand ÷ sram rand | 6.48 |
| **psram 1M rand ÷ psram 128K rand** | 1.18 |
| **flash 1M rand ÷ flash 128K rand** | 1.24 |

**The last two rows are the same memory, the same pattern, the same code. What changed,
and what does that say about a benchmark whose working set fits in the cache?**

> 唯一改變的是 working set 的大小，128 KB 變成 1 MB，也就是 32 KB 的 data cache 能裝下的比例：128 KB 時約 1/4，1 MB 時約 3%。所以隨機讀取時 PSRAM 慢了 1.18 倍、Flash 慢了 1.24 倍，循序讀取則幾乎不變（72.65 → 72.50、151.17 → 151.16），因為循序讀每條 cache line 本來就只付一次。
> 這表示 working set 越能放進 cache，量到的就越接近 cache 的速度，而不是記憶體本身的速度。一個 working set 完全放得進 cache 的 benchmark，量到的其實是 cache，會把慢的記憶體說得很快。
> 和我第 1 節 (c) 的預測方向一致（變慢），但幅度比我想的小很多：命中率從 1/4 降到 3%，時間只多了 18–24%，因為有一大部分時間是迴圈本身（見下一題）。

**Your `control_noload` figure is inside every `rand` figure. For the `sram_rand` row,
what fraction of the time was the memory and what fraction was the loop?**

> `sram_rand` 是 147.05 ns/word，其中 `control_noload`（只算隨機索引、不讀記憶體）就佔了 130.19 ns，也就是**迴圈約 88.5%，記憶體只有 147.05 − 130.19 = 16.86 ns，約 11.5%**。
> 這也修正了我第 1 節 (b) 的預測：SRAM 的 rand ÷ seq 是 5.00，不是「幾乎不變」，但多出來的幾乎全是迴圈的成本。扣掉 control 後，SRAM 隨機讀的記憶體成本（16.9 ns）甚至比循序（29.4 ns）還少，代表記憶體本身沒有變慢。同樣扣掉 control 後，PSRAM 128K 是 356 ÷ 72.65 ≈ 4.9 倍，Flash 128K 是 823 ÷ 151.17 ≈ 5.4 倍，Flash 確實變得最多。但如果直接看沒扣 control 的 rand ÷ seq，PSRAM（6.69）反而比 Flash（6.30）高，和我預測的不一樣。

---

## 3 · Run 2 · one dense layer, six ways

`weights_bytes_f32 = 131072` · `weights_bytes_s8 = 32768` (+ `4` bytes of
scale). These two are **computed from the shape**, not measured. Say so if you quote them.

| µs per inference (median) | SRAM | PSRAM | FLASH |
|---|---|---|---|
| float32 | 835.9 | 2229.7 | 4954.7 |
| int8 | 1242.3 | 1245.5 | 1246.8 |

`f32_outputs_identical = ok` · `s8_outputs_identical = ok` · `table_check = ok`

（兩個權重大小是依 256 × 128 的形狀算出來的，不是量測值。）

```
python scripts/ratios.py w11/run1-hierarchy.txt w11/run2-dense.txt
```

| Ratio | Value |
|---|---|
| f32 sram ÷ s8 sram (**width**, fast memory) | 0.67 |
| f32 flash ÷ s8 flash (**width**, slow memory) | 3.97 |
| f32 flash ÷ f32 sram (**tier**, wide weights) | 5.93 |
| s8 flash ÷ s8 sram (**tier**, narrow weights) | 1.00 |
| **f32 sram ÷ s8 flash** (wide-and-fast against narrow-and-slow) | 0.67 |

**Which bought more on your board — halving the bytes four times (int8) or moving the
bytes two tiers up (flash → SRAM)? Quote both ratios.**

> 在我的板子上，**把 float32 權重從 Flash 搬到 SRAM 換到的比較多：f32 flash ÷ f32 sram = 5.93 倍**；改成 int8 在 Flash 上換到 3.97 倍（f32 flash ÷ s8 flash），在 SRAM 上反而變慢（f32 sram ÷ s8 sram = 0.67，int8 1242 µs 對 float32 836 µs）。
> int8 在三種記憶體裡幾乎一樣快（1242.3 / 1245.5 / 1246.8 µs，s8 flash ÷ s8 sram = 1.00），代表 int8 這個 kernel 卡在**運算**而不是記憶體：每個 MAC 約 38 ns（約 9 個 cycle），只讀 1 byte，Flash 的讀取延遲完全被運算時間蓋過。它是純 C 的純量迴圈，要做 int8 的符號擴展和 32 位元整數乘法，比 FPU 的 float 乘加還慢。所以 int8 的好處是「讓權重放在哪裡都沒差」，以及省下 4 倍空間；要真的比 float 快，需要 `reading/` 裡那種 ESP32-S3 SIMD 組語 kernel，今天的純量 kernel 做不到。

### The error you paid

| | Value |
|---|---|
| `quant_rel_rms_err_pct` | 0.5468 % |
| `quant_max_abs_err` | 0.066963 |
| `quant_half_step_w` (the per-weight bound, s/2) | 3.936942e-03 |

**This is the error of one layer on one input. Name one reason it is not the accuracy
loss of a model.**

> 這只是**一層、一個輸入**的輸出誤差（相對 RMS 0.55%）。模型的準確率是很多層串起來之後，在整個測試集上**分類結果**有沒有改變：誤差會一層一層累積和放大，activation 也要量化，而 0.55% 的輸出誤差只有在剛好讓某個樣本跨過決策邊界時才會變成一個錯誤分類。另外這一層的權重是 `gen_tables.py` 用固定種子產生的，不是訓練過的模型，所以不能代表任何模型的準確率損失。W10 的 int8 模型要用同一批測試 window 重新跑一次混淆矩陣才知道。

---

## 4 · Run 3 · six rows per token

`bytes_per_token = 384` · `sram_holds_table = no (largest internal block 335860, table 1048576)`

| Test | table bytes | µs per token (median) | first-pass µs |
|---|---|---|---|
| `flash1m_lookup` | 1,048,576 | 14.91 | 15.04 |
| `psram1m_lookup` | 1,048,576 | 9.54 | 11.03 |
| `flash16k_lookup` (cache-resident control) | 16,384 | 2.84 | 4.82 |
| `psram16k_lookup` (cache-resident control) | 16,384 | 2.84 | 3.55 |
| `sram16k_lookup` (cache-resident control) | 16,384 | 2.84 | 2.83 |

```
python scripts/ratios.py w11/run1-hierarchy.txt w11/run2-dense.txt w11/run3-sparse.txt
```

| Ratio | Value |
|---|---|
| flash 1M lookup ÷ psram 1M lookup | 1.56 |
| flash 1M ÷ flash 16K (what the cache was hiding) | 5.24（PSRAM：3.36） |
| fraction of the 1 MB table touched per token (%) | 0.0366 %（384 / 1,048,576） |
| **s8 sram inference (run 2) ÷ flash 1M lookup (run 3)** | **83.32**（1242.3 µs ÷ 14.91 µs） |

**The last ratio is the whole argument of this week in one number. In a sentence: why
can a table that is 86 % of a model's parameters live in the slowest memory on the board
without being the thing that limits tokens per second?**

> 因為每個 token 只讀這張表的 6 列、384 bytes（1 MB 表的 0.0366%），就算放在最慢的 Flash，一次查表也只要 14.91 µs；而同一塊板子上，從 SRAM 跑一層 int8 dense 就要 1242.3 µs，是查表的 **83 倍**。每個 token 的時間幾乎全花在把 dense 權重全部讀一遍、算完，表放在哪裡對 tokens/s 幾乎沒有影響。

**The `16k` rows are the same code against the same memory. Why are they not a
measurement of that memory, and what would a student who only ran the 16 KB version have
concluded?**

> 16 KB 的表放得進 32 KB 的 data cache，第一輪之後每一列都是 cache hit，所以量到的是 cache 的速度：Flash、PSRAM、SRAM 三個 16K 測試的中位數**完全一樣，都是 2.84 µs**。只有冷的第一輪看得出差別（Flash 4.82、PSRAM 3.55、SRAM 2.83 µs）。真正 1 MB 的表在 Flash 上是 14.91 µs，是 16K 版本的 **5.24 倍**（PSRAM 是 3.36 倍）。
> 只跑 16 KB 版本的學生會得出「Flash、PSRAM 和 SRAM 一樣快，放哪裡都沒差」的結論，把 Flash 查表的成本低估 5 倍以上。

---

## 5 · The placement exercise

`EXERCISE_placement.md` — filled in and **handed in before the reveal**. Tick when done:
[x]（已填好 `EXERCISE_placement.md`；Moodle 由我自己上傳）

---

## 6 · The price of the build

Check these against your own Build output. These are the only numbers in this pack that
were not measured by you. **RAM should match to the byte on every desk; Flash will differ by tens of bytes**, because
the Arduino framework compiles its own source path — which runs through your Windows username — into the binary — a longer username, a larger figure. Write both of yours down.
**If RAM differs from the figure in the pack, that is a signal, not noise** — say by how much,
and say what you think is different about your build. Do not copy the deck's.

| | Flash (bytes) | RAM (bytes) |
|---|---|---|
| Sketch 1 · hierarchy — deck | 272,941 | 18,612 |
| Sketch 1 — **yours** | 272,925（比 deck 少 16） | 18,612（與 deck 相同） |
| Sketch 2 · dense — deck | 437,545 | 20,936 |
| Sketch 2 — **yours** | 437,529（比 deck 少 16；比 Sketch 1 多 164,604） | 20,936（與 deck 相同；比 Sketch 1 多 2,324） |
| Sketch 3 · sparse — deck | 270,209 | 18,596 |
| Sketch 3 — **yours** | 270,193（比 deck 少 16） | 18,596（與 deck 相同） |

**All three projects have the same three files in `src/`. Sketch 2's Flash figure is
about 164 KB larger than the other two. What is in those 164 KB, and why did sketches 1
and 3 not pay for them?** (`w11_tables.cpp` is in all three builds. Look up what
`--gc-sections` does to a const array nobody references.)

> 主要是 dense 層的權重表：`w11_tables.cpp` 裡的 `W11_W_F32_BITS[32768]`（32,768 × 4 = 131,072 bytes）和 `W11_W_S8[32768]`（32,768 bytes），加上 256 筆的輸入（`W11_X_F32_BITS` 1,024 bytes、`W11_X_S8` 256 bytes）和幾個常數，合計約 165 KB。我的 Sketch 2 比 Sketch 1 多 164,604 bytes，剩下的差額是 dense kernel 和報告程式碼本身，以及 Sketch 1 自己有而 Sketch 2 沒有的程式碼。
> Sketch 1 和 3 沒有付這筆，是因為 framework 用 `-fdata-sections` 把每個全域陣列編進各自的 section，再用 `-Wl,--gc-sections` 讓 linker 丟掉沒有任何程式參照的 section。Sketch 1、3 從來沒有用到這些陣列，所以就算 `w11_tables.cpp` 有編譯，陣列也被整個丟掉。我用 `xtensa-esp32s3-elf-nm` 檢查 Sketch 3 的 `firmware.elf`，找不到任何 `W11_W*` 符號。

**Sketch 2's RAM figure is about 2.3 KB above sketch 1's. Find the four static arrays
in `sketch_2_dense.cpp` and account for the difference.**

> Sketch 2 的 RAM 比 Sketch 1 多 2,324 bytes（20,936 − 18,612）。`sketch_2_dense.cpp` 第 50–53 行的四個 static 陣列：`float xF[256]` = 1,024、`int8_t xQ[256]` = 256、`float yF[128]` = 512、`int32_t yQ[128]` = 512，合計 **2,304 bytes**，佔了差值的 99%。剩下約 20 bytes 是其他小的全域變數（`f32Hash[3]`、`s8Hash[3]` 共 24 bytes、`f32Have[3]`、`s8Have[3]` 共 6 bytes），再扣掉 Sketch 1 自己有、Sketch 2 沒有的全域變數和對齊。權重本身不在 RAM：SRAM / PSRAM 版是執行時才用 malloc 從 heap 拿，Flash 版是 const，留在 Flash。

---

## 7 · Method note (one sentence, required)

> Timed with the CPU cycle counter on the board under test; median of five passes; no
> external instrument.

If anything about your run differed from that (a `cpu_mhz` other than 240, a refused
tier, a reduced SRAM working set), say so here:

> 和上面的方法相同：`cpu_mhz = 240`，沒有任何 tier 或測試被拒絕，SRAM 的 working set 是完整的 128 KB（`sram_words = 32768`），三個 run 的 `report.py` 都是 `VERDICT valid`。
> 不同的地方：(1) Run 3 的 log 抓到兩個 report block，第一個是燒錄完自動跑的，第二個是我按 RST 跑的。兩個除了位址和 checksum 之外完全相同，我只用第二個（`w11/run3-sparse.txt`），原始 log 保留為 `w11/run3-sparse_raw-two-blocks.txt`。(2) 這台電腦的 Windows 應用程式控制擋住了 `pio.exe`，build、上傳和 monitor 都改用 `penv\Scripts\python.exe -m platformio`，monitor 加了 `--dtr 0 --rts 0`，所以每個 run 都是我手動按 RST 開始。(3) 這週是線上週，實驗在 2026-11-26 完成，晚於 25 Nov 的期限。

## 8 · Judgement (one sentence, required)

**A model's weights are 1.2 MB in int8 and are read once per inference. Name the tier
you would put them in on this board, and defend it with your numbers** — a ratio from
section 3 or 4 in the sentence. "Flash is slow" is not an answer. "Flash sequential cost
X× SRAM on my board, and the 1 MB does not fit in a 512 KB SRAM in the first place" is.

> 我會放在 **Flash**：1.2 MB 根本放不進 SRAM（最大的內部連續區塊只有 335,860 bytes），而在我的板子上，int8 dense 層的權重放在 Flash 和放在 SRAM 幾乎一樣快（s8 flash ÷ s8 sram = 1.00，1246.8 µs 對 1242.3 µs），因為純量 int8 kernel 卡在運算而不是記憶體，所以把它搬去 PSRAM（s8 psram ÷ s8 sram = 1.00）只會多用掉 1.2 MB 的 PSRAM 和開機時的複製時間，什麼也換不到。
>
> 但如果改用 float32，或之後換成更快的 SIMD kernel，記憶體會重新變成瓶頸（f32 flash ÷ f32 sram = 5.93，Run 1 的 Flash 循序讀是 SRAM 的 5.14 倍），到時候就要重新量測，可能要改放 PSRAM（f32 psram ÷ f32 sram = 2.67）。

## 9 · The limit of today (one sentence, required)

**What could today's measurement not have seen?** (Who timed the board? With whose clock?
What was the cache doing during the first pass? What did the loop cost?)

> 所有時間都是**受測的板子自己**用自己的 CPU cycle counter 量的，沒有任何外部儀器（示波器或另一個時脈）可以確認那個時鐘和計時方法本身沒有偏差；每個測試的第一輪都是在沒人控制的 cache 狀態下跑的，中位數把它藏起來了，例如 flash16k 的第一輪 4.82 µs 對中位數 2.84 µs；而每個隨機讀取的數字裡都包含迴圈本身的成本（`sram_rand` 147 ns 中有 130 ns 是迴圈），所以今天看到的是「這段程式碼在這塊板子上的速度」，不是記憶體本身的存取時間，也沒有量到功耗。

---

## Before you leave

- [x] `run1-hierarchy.txt`, `run2-dense.txt`, `run3-sparse.txt` in `w11/`, each with a
      complete `=== W11 REPORT ===` block that `report.py` calls `valid`
- [x] Section 1 filled in **before** run 1, not after
- [x] Every refused figure written as `refused: <reason>`, not left blank, not guessed（這次沒有任何被拒絕的數字）
- [x] `report.py` and `ratios.py` committed, so your numbers can be reproduced
- [ ] `EXERCISE_placement.md` handed in **before the reveal**（已填好，Moodle 待我自己上傳）
- [x] Two entries in `AI_USAGE.md` — the int8 speed-up question and the arrhythmia prompt (carried over from W10)
- [x] This sheet, filled in

Files in the repo **by Wed 25 Nov, 23:59** (W11 is online this week). No HW is set this week; **section 3's six cells and
section 4's first ratio are the first real inputs to HW6's power budget in W12**, and you
cannot re-create them once the board is reflashed for something else.
