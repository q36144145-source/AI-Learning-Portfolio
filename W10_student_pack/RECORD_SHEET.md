# W10 Record Sheet

**Name** 陳竑勳  **Student ID** Q36144145  **Date** 2026-11-12

Every cell below is measured by you, in this room, today, or chosen by you from a table
your own board produced. **There is no reference answer on any slide in this deck, and none
in this folder.** If a cell is empty at 12:00, say so — an empty cell is a result, a guessed
cell is not.

The only pre-filled numbers anywhere in this pack are the Flash and RAM byte counts in
section 6, because the linker — not the board — produces them, and RAM comes out the same on
every desk in the room.

---

## 0 · Before you collect anything (slides 5 and 8)

**Spec first, two lines:**

| | |
|---|---|
| The decision this classifier would inform (e.g. "raise an alert when …") | 偵測到一個 `tap`（衝擊）window 時發出警示，作為 W08 跌倒手環「衝擊」偵測的替代練習。 |
| What would make me reject it (a recall, a cost, a latency — with a number) | 在另一個 session 上 `tap` recall 低於 87.5%（8 個 tap 漏掉超過 1 個），或單次推論超過 1,000 µs（取樣週期 10,000 µs 的 10%）。 |

（註：這兩行是收集資料之後才補寫的，不是事先寫好的規格。）

**Consent, three lines.** Tick each only when it is true:

- [x] I know what is collected: six axes at 100 Hz, two-second windows, no name or ID in the file.
- [x] I know who sees it: me, the grader, a neighbour **only if we both tick the next box**,
      and Edge Impulse (Qualcomm) **only if I upload**.
- [x] I agree it is deleted at the end of the semester, from my repo and from the Studio.
- [x] 補充（2026-11-12）：我同意把**我自己的** session 資料（A、B、live）放在公開的 GitHub 作品集；同學的資料不公開。
- [x] **Swap:** I give my session A to 同學 L（公開版隱去姓名） and receive theirs. Both signed: 陳竑勳 / 同學 L（同學的 session A 存為 `w10/neighbour_sessionA.txt`，用於 §5 的 neighbour-split 補充測試；主要測試集仍為自己的 session B）

---

## 1 · The data (slides 12–14)

| | Session A (train) | Session B or neighbour's (test) |
|---|---|---|
| Log file name | `w10/sessionA.txt` | `w10/sessionB.txt` (own session B) |
| Windows kept, total | 32 | 32 |
| still / tilt / shake / tap kept | 8 / 8 / 8 / 8 | 8 / 8 / 8 / 8 |
| Windows dropped, and the reasons (copy `windows.py`'s lines) | 0 dropped | 0 dropped |
| Always-majority accuracy on the **test** set (printed by `windows.py`) | — | 25.0% (always 'still') |

**Did any `tap` window report `clipped` ≠ 0?** No — all 64 windows (A and B) report `missed=0 clipped=0 read_failed=0`. (Closest call: session A window 7, tap, `gmag_max` = 514.7 dps; the magnitude exceeds 500 but no single axis reached full scale, so it was not counted as clipped.) If so, first say **which sensor**: look at
`amag_max` and `gmag_max` on that window. An `amag_max` well under 4 g with `gmag_max` past
500 dps means the gyroscope clipped, not the accelerometer — and raising the accelerometer
range would not have helped. If it is the accelerometer: which two constants would have to
change **together** to raise the range, and where does each come from?

> 不適用：沒有任何 window 被 clipped。若要提高加速度計量程，必須同時修改兩個常數：`ACCEL_CFG_VALUE`（register map 的 ACCEL_CONFIG 暫存器 AFS_SEL 欄位）和 `XL_SENS_MG`（datasheet 中對應量程的靈敏度）。只改其中一個，數值的單位就會錯。

**For every window, the label came from the prompt. Name one window where you did not do
what it said, or say honestly that you cannot know.**

> 就我所知，A、B 兩個 session 的每個 window 都照提示做了，沒有刻意做錯任何一個。但標籤只來自提示，沒有第三方核對，所以我無法證明每個動作都「標準」，例如 tilt 是否真的轉到 90°。唯一可疑的是 session A 的 window 6（still）：`gmag_max` 是 10.0 dps，其他 still 約 4 dps，可能手離開時稍微碰到。

---

## 2 · The feature table and your three numbers (slide 15)

From `w10_out/feature_table.txt` — **training session only**. Copy the min and max.

| Feature | still min / max | tilt min / max | shake min / max | tap min / max | Your threshold |
|---|---|---|---|---|---|
| `amag_max` (g) | 0.957 / 0.965 | 1.290 / 1.923 | 1.531 / 2.333 | 1.803 / 3.104 | `T_TAP_G` = 2.0 |
| `amag_std` (g) | 0.002 / 0.003 | 0.093 / 0.144 | 0.157 / 0.321 | 0.094 / 0.184 | `T_SHAKE_G` = 0.15 |
| `gmag_mean` (dps) | 3.039 / 4.336 | 54.442 / 123.636 | 37.355 / 80.506 | 21.878 / 42.123 | `T_TILT_DPS` = 30 |

**Did any two class ranges overlap on the row you chose from?** Which, and by how much?
That overlap is a finding; the model gets to show whether it can do what one number cannot.

> 有，而且最嚴重的是 `amag_max` 這一列（`T_TAP_G` 的依據）：tap（1.803–3.104 g）和 shake（1.531–2.333 g）在 1.803–2.333 g 重疊，寬約 0.53 g；tilt（1.290–1.923 g）也和 tap 在 1.803–1.923 g 重疊約 0.12 g。不管 `T_TAP_G` 選多少，都沒辦法在訓練集上同時把 tap 全抓到、又不把 shake 誤判成 tap。我選 2.0 g 時，訓練集有 3 個 tap 低於門檻，有 4 個 shake 高於門檻。
> 另外兩列：`amag_std` 上 tilt 最大 0.144、shake 最小 0.157，中間有一小段空隙，所以選 0.15；但 tap（0.094–0.184）和 tilt、shake 兩邊都重疊。`gmag_mean` 上 still 最大 4.3 dps、tilt 最小 54.4 dps，有很大的空隙，所以選 30；但 tap（21.9–42.1）和 shake（37.4–80.5）在 37.4–42.1 dps 重疊。

The same three numbers go into `baseline.py`'s arguments and into `sketch_c_classify.cpp`.
**Three places, one set of numbers.**

---

## 3 · The rule's matrix (slides 16–17)

From `baseline.py`, the **TEST** block only. Rows are what you did, columns what it said.

| RULE · test | said still | said tilt | said shake | said tap | recall | (1/n = 12.5 pts) |
|---|---|---|---|---|---|---|
| did still | 8 | 0 | 0 | 0 | 100.0% | 1/8 |
| did tilt | 0 | 8 | 0 | 0 | 100.0% | 1/8 |
| did shake | 0 | 0 | 5 | 3 | 62.5% | 1/8 |
| did tap | 3 | 0 | 0 | 5 | 62.5% | 1/8 |
| **precision** | 73% | 100% | 100% | 62% | | |

| | |
|---|---|
| Overall accuracy | 81.2% (26 / 32) |
| Always-majority baseline (same test set) | 25.0% (always 'still') |
| Largest off-diagonal cell (did __, said __, × __) | did shake, said tap, × 3 (tied with did tap, said still, × 3) |
| The **costliest** cell, in your judgement, and why it is not the same one | did tap, said still ×3。數量上最多的錯誤是 shake→tap（×3，和這格同數），但那只是把「有動作」誤認成另一種「有動作」，頂多造成一次誤報。tap→still 則是真的發生了衝擊事件，規則卻回報「什麼都沒發生」，屬於漏報。如果套用到 W08 的跌倒手環，衝擊就像跌倒的那一下，漏報的代價（沒有人去救）遠大於誤報（多響一次）。 |

---

## 4 · The model's matrix, same test windows (slides 27 and 34)

**Which path?**  [x] `train.py` offline (required)   [x] Edge Impulse Studio as well   [ ] Studio unavailable: ______________

| MODEL (`train.py`) · test | said still | said tilt | said shake | said tap | recall |
|---|---|---|---|---|---|
| did still | 8 | 0 | 0 | 0 | 100.0% |
| did tilt | 0 | 8 | 0 | 0 | 100.0% |
| did shake | 0 | 0 | 8 | 0 | 100.0% |
| did tap | 0 | 0 | 0 | 8 | 100.0% |
| **precision** | 100% | 100% | 100% | 100% | |

Overall accuracy 100.0% (32 / 32)  Majority baseline 25.0%  `train.py` epochs 400 seed 0  (model = 176 float32 = 704 bytes)

**Studio, if used** (Model testing → Classify all, Testing category). Epochs 30 learning rate 0.0005（預設值未修改；Dense 20 → Dense 10；訓練資料保留 20% 做驗證；minimum confidence 0.6）

Impulse：Time series 2000 ms window、2000 ms increase、100 Hz → Spectral Analysis（六軸 accX…gyrZ）→ Classification。透過 Studio API 建立與執行，步驟和網頁版相同。

| STUDIO · test（取最高分的類別，不套 0.6 信心門檻） | said still | said tilt | said shake | said tap |
|---|---|---|---|---|
| did still | 8 | 0 | 0 | 0 |
| did tilt | 0 | 0 | 6 | 2 |
| did shake | 0 | 0 | 0 | 8 |
| did tap | 0 | 0 | 0 | 8 |

準確率 50.0%（16/32）。Studio 頁面上的 Model testing 會套 0.6 的信心門檻，顯示為 **43.75%**（14/32）：still 有 2 個、tilt 有 2 個被標成 uncertain。Studio 自己的驗證集（訓練資料保留的 20%，7 個 window）準確率 42.9%，loss 7.08。
> 判讀：同樣的 32 個訓練 window，Studio 的預設設定（30 epochs、lr 0.0005）明顯訓練不足；`train.py` 是 400 epochs、lr 0.05，而且有先標準化 8 個特徵。另外，Studio 用 Spectral features 加 20% 驗證切分，實際只用了約 25 個 window 訓練。Studio 模型的 tilt 和 shake 全錯（0/8），而且全部被說成 shake 或 tap。這個差異來自設定和特徵，不代表 Edge Impulse 這個工具本身比較差；講義也說今天的重點在流程，不調超參數。

---

## 5 · Rule against model (slide 35)

```
python scripts/compare.py w10_out/cm_rule.json w10_out/cm_model.json
       [--manual "a,b,c,d;e,f,g,h;i,j,k,l;m,n,o,p" --manual-name edge-impulse]
```

| recall | rule | model (offline) | model (Studio) | model − rule, points | one window = 12.5 pts |
|---|---|---|---|---|---|
| still | 100.0% | 100.0% | 100.0% | +0.0（Studio +0.0） | 0 windows |
| tilt | 100.0% | 100.0% | 0.0% | +0.0（Studio −100.0） | 0（Studio −8） |
| shake | 62.5% | 100.0% | 0.0% | +37.5（Studio −62.5） | 3（Studio −5） |
| tap | 62.5% | 100.0% | 100.0% | +37.5（Studio +37.5） | 3（Studio +3） |
| accuracy / majority | 81.2% / 25.0% | 100.0% / 25.0% | 50.0% / 25.0%（套 0.6 門檻為 43.75%） | +18.8（Studio −31.2） | 6 of 32（Studio −10） |

**Fill the last column first.** A difference smaller than one window is not a difference.

**On which class, if any, did the model change the verdict by more than one window — and is
that the class whose ranges overlapped in section 2?**

> 有，shake 和 tap 兩類。各 8 個測試 window，一個 window 等於 12.5 點；兩類都從 5/8 進步到 8/8，各多對 3 個 window（+37.5 點），超過一個 window 的步長。still 和 tilt 規則本來就 8/8，模型沒有改變（+0）。改變的正是第 2 節裡範圍重疊的兩類：tap 和 shake 在 `amag_max` 上重疊，一個特徵配一個門檻分不開，模型同時用 8 個特徵就分開了。不過每類只有 8 個 window、資料來自同一個人和同一個下午，100% 只代表在這 32 個 window 上沒有錯，不代表換一個人或換一天也一樣。

**補充測試：換成同學的 session（neighbour-split）**。訓練資料不變，仍是我的 session A、同一組門檻 2.0 / 0.15 / 30、權重和燒錄的模型完全相同；測試集換成同學的 session A（`w10/neighbour_sessionA.txt`；依交換約定，同學的原始資料與特徵檔不放在公開 repo，只保留結果數字；32 個 window 全保留，8/8/8/8），輸出在 `w10_out_neighbour/`。

| recall · 同學的 session | rule | model (offline) | model − rule | one window = 12.5 pts |
|---|---|---|---|---|
| still | 87.5% (7/8) | 87.5% (7/8) | +0.0 | 0 windows |
| tilt | 25.0% (2/8) | 12.5% (1/8) | −12.5 | −1 window |
| shake | 25.0% (2/8) | 0.0% (0/8) | −25.0 | −2 windows |
| tap | 0.0% (0/8) | 37.5% (3/8) | +37.5 | +3 windows |
| accuracy / majority | 34.4% / 25.0% | 34.4% / 25.0% | +0.0 | 0 |

> 換一個人，兩者都掉到 34.4%，只比「永遠猜 still」的 25% 高一點，模型也不再贏規則。規則最大的錯是 did tap, said still ×8：同學的 tap 最大只有 1.945 g，從來沒超過 2.0 的門檻。模型最大的錯是 did shake, said tap ×5。
> 原因至少有兩個：
> 1. **安裝方向不同**：同學靜止時重力在 z 軸（+0.95 g），我的在 x 軸（−0.95 g），所以 `az_std` 在兩邊量的不是同一個方向。
> 2. **動作的力道和方式不同**：同學 shake 的 `amag_std` 只有 0.034–0.219（我的 0.157–0.321），tap 的 `amag_max` 0.978–1.945（我的 1.803–3.104）。
>
> 同學的資料裡，有些 tilt 和 tap window 的 `amag_std` 低到 0.003，和 still 一樣，代表那 2 秒裡可能幾乎沒有動作。所以我無法完全分辨「換人」和「同學的某些標籤不準」各占多少影響。

---

## 6 · The price (slides 38, 42, 45)

Check the first four cells against your own build output. **RAM should match to the byte.**
Flash moves with the length of the provenance string in `model_weights.h` — tens of bytes —
write yours and say by how much. (It does not move with your project path; that was checked.)

| | RAM (bytes) | Flash (bytes) | model bytes | `feat_us` | `rule_us` | `mlp_us` |
|---|---|---|---|---|---|---|
| Sketch A · collect — deck (5 Oct 2026, `espressif32@7.1.3`) | 23,648 | 292,649 | — | — | — | — |
| Sketch A — **yours** (countdown changed 3 s → 5 s; capture unchanged) | 23,648 | 292,601 (−48 vs deck) | — | — | — | — |
| Sketch C · placeholder header — deck | 23,700 | 295,069 | — | — | — | — |
| Sketch C · **your** header — **yours** (window 1) | 23,700 | 295,101 (−32 vs deck 295,133) | 704 | 229 | 4 | 79 |
| Sketch C — window 2 | 23,700 | 295,101 | 704 | 213 | 3 | 29 |
| Sketch C — window 3 | 23,700 | 295,101 | 704 | 216 | 3 | 29 |

**C minus A in RAM, yours: 52 bytes** (23,700 − 23,648). Both sketches hold one window buffer — 200 × 6 × 4 =
4,800 bytes by arithmetic on the struct — so the window is **not** in that difference. What is?
And the model's 704 bytes: which column of this table did they land in, and why that one?

> 用 `xtensa-esp32s3-elf-nm` 比對兩個 firmware.elf 的符號，多出的 52 bytes 剛好是：`feat[8]` 32 bytes（.bss）、三個門檻 `T_TAP_G`/`T_SHAKE_G`/`T_TILT_DPS` 共 12 bytes、`modelReady` 4 bytes，以及 `expf()` 從 libm 帶進來的 `__fdlib_version` 4 bytes，合計 32+12+4+4 = 52。window 緩衝區 `win` 在兩個程式裡都是 0x12D0 = 4,816 bytes（4,800 + 4 個 uint32 計數器），所以不在差值裡。
> 模型的 704 bytes 在 **Flash** 欄。`MODEL_*` 陣列都是 `static const`，位址在 0x3C0xxxxx（Flash 經 cache 映射的唯讀區），推論時直接從 Flash 讀，不複製到 RAM。所以 RAM 在 placeholder 和訓練後的 header 之間完全不變；placeholder 本身也有同樣大小的陣列，Flash 也只因 provenance 字串的長度而變動。

**`mlp_us` against the 10,000 µs sampling period: 0.29 %** (29 µs; window 1 was 79 µs = 0.79 %, the first call). Is the model "small"? Say
compared with what.

> 和「時間預算」相比很小：29 µs 只佔取樣週期的 0.29%、一個 2 s window 的 0.0015%，甚至比算 8 個特徵本身（約 214 µs）還快約 7 倍；704 bytes 也只佔 Flash 3,342,336 bytes 的 0.02%。但和「規則」相比不小：規則只要約 3 µs、不需要任何權重，模型慢約 10 倍，還多 704 bytes。所以「小」要看跟什麼比；以這塊板子的資源來說，模型的成本可以忽略。

---

## 7 · The live sheet (slides 43–44)

Twenty windows, five per class, **your order**. Write the truth **before** GO.

| # | truth (before GO) | rule said | model said | p | missed/clipped |
|---|---|---|---|---|---|
| 1 | tap | tap | tap | 0.99 | 0/0 |
| 2 | still | still | still | 1.00 | 0/0 |
| 3 | shake | shake | shake | 0.98 | 0/0 |
| 4 | tilt | tilt | tilt | 0.99 | 0/0 |
| 5 | shake | tap | shake | 1.00 | 0/0 |
| 6 | tap | tap | tap | 0.96 | 0/0 |
| 7 | tilt | tilt | tilt | 0.97 | 0/0 |
| 8 | still | still | still | 0.99 | 0/0 |
| 9 | still | still | still | 1.00 | 0/0 |
| 10 | shake | tap | shake | 1.00 | 0/0 |
| 11 | tap | still | tap | 0.98 | 0/0 |
| 12 | tilt | tilt | tilt | 0.98 | 0/0 |
| 13 | tilt | tilt | tilt | 0.93 | 0/0 |
| 14 | tap | tap | tap | 1.00 | 0/0 |
| 15 | still | still | still | 0.99 | 0/0 |
| 16 | shake | tap | shake | 0.98 | 0/0 |
| 17 | shake | tap | shake | 0.99 | 0/0 |
| 18 | still | still | still | 1.00 | 0/0 |
| 19 | tilt | tilt | tilt | 0.98 | 0/0 |
| 20 | tap | tap | tap | 0.98 | 0/0 |

| you did (5 each) | rule agreed | model agreed | both wrong — said what? | windows excluded (missed/clipped ≠ 0) |
|---|---|---|---|---|
| still | 5 / 5 | 5 / 5 | — | 0 |
| tilt | 5 / 5 | 5 / 5 | — | 0 |
| shake | 1 / 5 (said tap ×4: #5, #10, #16, #17) | 5 / 5 | none | 0 |
| tap | 4 / 5 (said still ×1: #11) | 5 / 5 | none | 0 |

**Is the live agreement much higher than section 5? If so, what is the first suspect?**
(Slide 44 names it.)

> 沒有更高。即時測試每類 5 個 window，一個 window 等於 20 點（第 5 節是一個 window 等於 12.5 點）。模型即時 20/20，和第 5 節的 32/32 一樣全對，沒有可以再提高的空間。規則即時 15/20（75%），反而比第 5 節的 26/32（81.2%）低：shake 只有 1/5（20%，第 5 節是 62.5%），tap 4/5（80%，第 5 節是 62.5%），still 和 tilt 都是 5/5。錯法和第 5 節相同：shake 被說成 tap ×4、tap 被說成 still ×1。
> 第一嫌疑：第 44 張投影片指出，即時 window 是在我已經看過幾次兩者的判斷之後才做的，我可能不自覺地調整動作，做出「能得到想要答案」的動作，讓一致率虛高。我在即時測試時確實看得到每個 window 的 `L,` 判斷行。但結果沒有出現這種跡象：規則的 shake 一致率反而下降了 2 個以上的 window，表示我沒有把 shake 搖得比較輕來避開 tap 門檻。不過 truth 和動作都出自我自己，即時資料和訓練資料也是同一個人、同一個下午，所以這個嫌疑沒辦法完全排除。

---

## 8 · Method note (one sentence, required)

> Two sessions from one board, one subject, session-split (A = train, B = test, board put down and picked up in between), plus a neighbour-split check (train = my A, test = a classmate's session A, swapped with consent; see §5);
> features and both classifiers computed on the board; timings self-measured with `micros()`.

Anything else a reader would need to repeat it, including what did **not** work:

> - Sketch A 的倒數由 3 秒改為 5 秒，原本的 3 秒來不及反應；錄製長度（2 s、200 samples、100 Hz）不變。Sketch C 維持原本的 3 秒。改過的程式保存在 `w10/sketch_a_collect_5s_countdown.cpp`。
> - GY-521 在麵包板上是立著插的，平放時重力落在 x 軸（約 −0.95 g），不是 z 軸。A、B、live 三次都用同樣的擺法。因此 `az_std`（最能分開 tap 和 shake 的特徵：A 中 shake 0.499–0.821、tap 0.113–0.187）量的是一個水平軸，結果綁定這個安裝方向，換一種裝法要重新驗證。
> - 五個常數：WHOAMI 0x68、ACCEL_CFG 0x08（±4 g）、GYRO_CFG 0x08（±500 dps）、XL_SENS_MG 0.1220703125（沿用 W05）、G_SENS_MDPS 15.2672（= 1000 / 65.5 LSB/°/s）。
> - 沒有成功的地方：用 PlatformIO monitor 開 COM7 時，板子會被 DTR/RTS 自動重置，還沒按 RST 就開始錄。後來改用 `pio device monitor --dtr 0 --rts 0`。中途中斷的兩份 log（10 個與 4 個 window）已刪除，沒有拿來用。
> - Build：RAM 和講義相同（A 23,648 / C 23,700）；Flash 分別少 48 與 32 bytes（A 292,601、C 295,101）。環境為 Python 3.14、`train.py` 400 epochs、seed 0。
> - Edge Impulse：`ei_export/training` 32 個、`testing` 32 個檔案，用 ingestion API 逐檔上傳，每檔明確指定標籤、不用自動切分；專案確認 Training 32、Testing 32 筆。Impulse、特徵產生、訓練、Classify all 都透過 Studio API 執行（臨時 Admin key，用完即刪），設定見 §4，訓練用預設值。
> - 同學的 session 用相同的五個常數格式（`xl_sens_mg=0.1220`、`g_sens_mdps=15.2670`，和我的差不到 0.02%），但安裝方向不同（重力在 z 軸）。

## 9 · Judgement (one sentence, required)

**For the W08 fall band, would you ship the rule or the model — and why, with the section 5
difference, the section 6 microseconds and the section 7 live agreement in the sentence.**
"The model is more accurate" is not an answer.

> 我會選模型，但只拿去做下一階段的測試，不是直接給使用者用。在同一批測試 window 上，模型在 shake 和 tap 各多對 3 個 window（各 +37.5 點，超過一個 window 的 12.5 點步長），而且正好是規則因 `amag_max` 重疊而分不開的兩類；即時測試模型 20/20、規則 15/20，規則還漏了 1 個 tap（說成 still）。代價是每次推論約 29 µs（取樣週期的 0.29%）加上 704 bytes Flash，規則約 3 µs，這個代價對手環來說可以忽略。
>
> 不過，按照第 0 節的標準，規則在測試集上的 tap recall 只有 62.5%，已經不合格；模型雖然通過，但只是 8 個 tap 全對，樣本太少，不足以證明它在真實跌倒上可靠。而且換成同學的 session，兩者都掉到 34.4%，模型的 tap recall 只剩 3/8（37.5%），規則是 0/8，兩者都不合格。所以現在不管選哪一個都不能出貨；下一步是收集多人、多種安裝方向的資料，重新訓練與驗證。

## 10 · The limit of today (one sentence, required)

**What can today's data not support?** (Slide 48 lists four things; name the one that
matters most for the claim in section 0.)

> 今天的資料無法支持「能偵測跌倒」這個說法，因為 `tap` 是我拿著麵包板在桌邊敲出來的**模擬動作，不是真實事件**。真實跌倒的衝擊大小、方向、前後的動作都不同，而這些資料又只來自一個人、一張桌子、一個下午、由我自己起止的 2 秒 window。所以模型的 100% 只代表它分得出「我今天做的這四種手勢」。換成同學的 session 之後，準確率立刻掉到 34.4%，直接證明了「單一受試者、單一安裝方式」是今天最大的限制之一。

---

## Before you leave

- [x] `sessionA.txt`, `sessionB.txt` (or the neighbour's, named), `live.txt` in `w10/`
- [x] `w10_out/` complete: `features_*.csv`, `feature_table.txt`, `manifest.json`, `cm_rule.json`, `cm_model.json`
- [x] `model_weights.h` as flashed, in `w10/`
- [ ] Section 0 filled in **before** section 1（未做到：第 0 節是事後補寫的，已在第 0 節註明）
- [x] Section 2 chosen from the **training** session only
- [x] Section 7 truth column written **before** each GO
- [x] Two entries in `AI_USAGE.md` — the arrhythmia prompt, and the same question about your own data
- [x] This sheet, filled in
- [ ] Google Form submitted **before you leave the room**

Files in the repo the same day. HW5 is built on every section of this sheet.
