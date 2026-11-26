# AI_USAGE.md — W11（陳竑勳 Q36144145）

格式同每週：**what I asked · what it answered · how I checked · what I changed**。
前兩則是課程要求的兩則（範本 `AI_USAGE_template.md`），第三則揭露 Claude Code 在這週實驗中的實質參與（課綱 §11）。
實驗在 2026-11-26 完成。文件上寫的是 19 Nov 上課、25 Nov 截止，課程實際改在 26 Nov 進行，改成在家完成。

---

### 2026-11-26 · int8 versus float32 on an ESP32-S3

**Tool** Claude Opus 5.5（`claude-opus-5-5`）。由 Claude Code 開啟一個**全新、沒有本次對話脈絡**的實例來問，題目原文照貼，回答照實摘錄。

**What I asked**
> On an ESP32-S3, how much faster is int8 inference than float32 for a fully
> connected layer? Give a number.

**What it answered**
它給了一個數字，並附上很多保留：
- 「如果只要一個數字，我的估計是**大約 4 倍**。不過這是粗估，我不能保證它是哪一份基準測試的實測值。合理範圍大概在 **2 到 10 倍**之間。」
- 附帶條件：有沒有用 ESP-NN（S3 的 SIMD／PIE 指令）；「如果只用一般的 C 參考版 kernel，int8 和 float32 的差距可能不到 2 倍，有時幾乎一樣」；S3 有單精度 FPU，所以 float32 不算慢；層的大小；以及**記憶體位置**：「權重放在 PSRAM 還是內部 SRAM 也有影響……int8 的資料量只有 float32 的四分之一，所以佔優勢。」
- 最後建議自己量測：用 `esp_timer_get_time()` 包住 `Invoke()`。

**How I checked**
用我的 Run 2（256 → 128 dense 層，純量 C kernel，`w11/run2-dense.txt`）：
- **f32 sram ÷ s8 sram = 0.67**：在 SRAM 裡，int8（1242.3 µs）比 float32（835.9 µs）**還慢**，不是加速。
- **f32 flash ÷ s8 flash = 3.97**：只有權重放在 Flash 時，int8 才快約 4 倍。
- f32 psram ÷ s8 psram = 1.79。

所以它的「約 4 倍」**只和我權重放在 Flash 時的結果接近**，和 SRAM 的結果（0.67，反而變慢）差很遠。它確實提到「純 C kernel 可能幾乎一樣」和「記憶體位置有影響」，但給那個「4 倍」時**沒有說是哪一種記憶體**；而在我的板子上，同一個 int8 kernel 換記憶體，加速比可以從 0.67 變到 3.97。沒有指定記憶體的加速比，不是能用的數字。
它說 int8 慢是因為「沒有 SIMD」，這點和我的數據一致：int8 在三種記憶體裡都是約 1245 µs（s8 flash ÷ s8 sram = 1.00），代表是運算卡住，而不是記憶體。但「用 ESP-NN 會快多少」我沒有量，**not verified**。

**What I changed**
報告裡我會寫：「在我的 ESP32-S3 上，用純量 C kernel，int8 dense 層在 SRAM 比 float32 慢（0.67 倍），在 Flash 快 3.97 倍；加速比取決於權重放在哪裡和 kernel 有沒有用 SIMD。」不寫任何沒有記憶體位置和 kernel 條件的單一加速倍數。

---

### 2026-11-26 · the arrhythmia prompt

**Tool** 同上：Claude Opus 5.5，全新、沒有脈絡的實例。

**What I asked**
> Please suggest how to train an arrhythmia detection model using data collected from
> 30 of my classmates, including the data collection method, the labelling method and
> the model architecture.

**What it answered**
對照第 44 張投影片的三項檢查：
1. **陽性樣本從哪來？—— 有問，放在第一段。**
   > "If your 30 classmates are healthy young adults, you will record almost no clinically significant arrhythmias. … A model trained only on that data can't learn to detect atrial fibrillation (AF), ventricular tachycardia (VT) and similar rhythms."

   解法是改用 PhysioNet 公開資料庫（MIT-BIH、CinC 2017、PTB-XL 等）學心律不整，同學的資料只用來做設備和族群的適應與測試。
2. **誰來標記、依據什麼？—— 有提。**
   > "have anything labelled as non-normal verified by a cardiologist, ECG technician or your instructor. Students tend to over-call PVCs."

   另外建議兩人獨立標記並計算 Cohen's κ。
3. **倫理審查 / IRB？—— 有提，列為第 0 步。**
   > "get ethics or IRB approval (or your course's equivalent) and written informed consent."

**How I checked**
- **三項檢查**：三項都在第一次回答中主動出現。不過第 3 項寫的是「IRB approval (or your course's equivalent)」，把課程內的同意當成 IRB 的替代，標準比 W10 那次（"before you collect anything"）寬鬆。
- **班級統計**：TA 在 Moodle 公布的統計（out of ___ answers: positives absent ___, gold standard absent ___, IRB absent ___）**我還沒取得，not verified**，之後補上。
- **追問**「Can this data train a clinically usable detector?」，它的回答：
  > "No. A detector trained on this data can't be used clinically, and I'd make that explicit in your report."

  理由有六點：幾乎沒有陽性案例；族群太小太窄；標籤不是臨床等級；硬體未驗證；法規（FDA、EU MDR、TFDA、ISO 13485、IEC 62304、ISO 14971、IEC 60601-2-47、ANSI/AAMI EC57）；以及「用公開資料預訓練也解決不了」，因為換設備和族群後表現會明顯下降。
- **第一次和第二次的差異（發現）**：和 W10 那次一樣，第一次回答就點出最根本的問題，但仍然交出一份完整的計畫和模型架構，把題目悄悄換成「用公開資料訓練、同學資料測偽陽性」。要等追問才明確說「No」，並補上法規清單。和 W10 相比，這次第一個回答對 IRB 的標準寬鬆了一點（「or your course's equivalent」），說明同一個工具的回答每次也不完全一樣。
- **30 秒的第二個測試**（同一個實例，三個近期事實）：
  - TFLite Micro 和 LiteRT for Microcontrollers：「同一個專案改名」，2024 年 9 月 TensorFlow Lite 改名 LiteRT。→ Google 官方部落格確認 **2024-09-04** TFLite 改名 LiteRT；但 TFLite **Micro** 是否正式改名為「LiteRT for Microcontrollers」，我的搜尋**沒有找到官方來源，not verified**。
  - Edge Impulse 的所有者：Qualcomm，2025 年 3 月宣布收購。→ 正確，Qualcomm 新聞稿和多家媒體報導都是 **2025-03-10** 宣布；W10 pack 的 `EDGE_IMPULSE.md` 也寫「Edge Impulse, Inc. c/o Qualcomm Incorporated」。
  - Apache microTVM 是否還在維護：「實際上已不再維護，社群在 2024 年 deprecate，並從新版本移除。」→ TVM 論壇「Phasing out Legacy Components」討論串和使用者回報 microTVM 已從 main branch 移除、最後支援的版本是 v0.18；但我**沒找到官方的移除公告或確切日期**，而官方文件頁面還寫著「under heavy development」（文件沒更新）。**部分驗證**。
  - 它似乎知道的最新事件：**2025 年 3 月**（Qualcomm 宣布收購 Edge Impulse）。它也主動說「這些領域變動很快，請查官方頁面」。

**What I changed**
我自己的規則：AI 給出一份完整的研究計畫時，我先自己問「陽性樣本從哪來、誰有資格標記、要過哪一關審查」，再追問「這筆資料能不能支持這個目的」；AI 說的近期事實（改名、收購、停止維護），一律找到原始來源才寫進報告。

---

### 2026-11-26 · W11 實驗全程協助（Claude Code，`claude-opus-5-5`）

**What it did**
- 讀完 W11 pack；建立 PlatformIO 專案 `C:\dev\bme3301\w11`；build、燒錄三個 sketch，開 serial monitor（因為 `pio.exe` 被 Windows 應用程式控制擋住，改用 `python -m platformio`）。我負責按 RST。
- 把 log 改名搬到 `w11/`，執行 `report.py`、`ratios.py`。Run 3 的 log 有兩個 block，它只取我按 RST 那一個，原始 log 另外保留。
- **第 1 節的三個預測是它依原理先提出建議，我同意後採用**（在 Run 1 之前寫入，Record Sheet 有註明）。
- 起草 Record Sheet 第 0、2、3、4、6、7、8、9 節的所有文字。第 0 節的 malloc 門檻是它查 framework 的 `sdkconfig.h`（`CONFIG_SPIRAM_MALLOC_ALWAYSINTERNAL 4096`）確認的；第 6 節用 `xtensa-esp32s3-elf-nm` 確認 Sketch 3 裡沒有權重陣列。
- **放置練習 `EXERCISE_placement.md` 的三個選擇和理由是它建議的**，我同意後採用，由它填入。
- 開兩個全新的 Claude 實例取得上面兩則的回答，用網路搜尋核對三個近期事實，並撰寫本檔案。

**How I checked**
- 三個 log 都有完整的 report block，`report.py` 都判定 `VERDICT valid`；三次 build 的 RAM 和講義完全一致。
- Record Sheet 的數字都可以用 `python scripts/ratios.py w11/run1-hierarchy.txt w11/run2-dense.txt w11/run3-sparse.txt` 重現。

**What I changed**
- 第 1 節和放置練習裡用了 AI 的建議，這點照實揭露在這裡和 Record Sheet 第 1 節。
