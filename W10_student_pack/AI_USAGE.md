# AI_USAGE.md — W10（陳竑勳 Q36144145）

格式同每週：**what I asked · what it answered · how I checked · what I changed**。
前兩則是課程要求的兩則（範本 `AI_USAGE_template.md`）；第三則揭露今天 Claude Code 在整個實驗中的實質參與（課綱 §11）。

---

### 2026-11-12 · the arrhythmia prompt

**Tool** Claude Opus 5.5（`claude-opus-5-5`）。由 Claude Code 開啟一個**全新、沒有本次對話脈絡**的 Claude 實例來問，題目原文照貼、不加任何提示；回答照實摘錄，未改寫。

**What I asked**
> Please suggest how to train an arrhythmia detection model using data collected from
> 30 of my classmates, including the data collection method, the labelling method and
> the model architecture.

**What it answered**
它給了完整計畫（倫理、蒐集流程、AAMI EC57 標註、公開資料庫預訓練、1D CNN / 1D ResNet 架構、以受試者分組的評估），但**第一段就主動質疑前提**。對照第 31 張投影片的三項檢查：

1. **陽性樣本從哪來？—— 有問，而且放在最前面。**
   > "there's one problem to plan around before anything else: **30 healthy university students will give you almost no arrhythmia data.** … A model trained only on your classmates would learn to say "normal" every time."

   它的解法是改用 PhysioNet 公開資料庫（MIT-BIH、CinC 2017 等）訓練，同學資料只拿來做適應與偽陽性測試。
2. **誰來標記、依據什麼？—— 有提。**
   > "you need a cardiologist, cardiac physiologist, or ECG-trained clinician to confirm anything non-normal. You and your classmates aren't qualified to call a PVC versus an aberrant beat."

   另外建議兩人重複標註 10–20% 的資料，並計算 Cohen's kappa。
3. **倫理審查 / IRB？—— 有提，列為第 0 步。**
   > "**Get ethics approval** from your university's ethics committee (IRB/REC) before you collect anything."

   也提到知情同意、假名化，以及錄到疑似異常時該怎麼處理（incidental findings）。

**How I checked**
- **三項檢查**：三項都在第一次回答中主動出現，不是追問後才補上。
- **類別計數**：它自己點出這個問題，30 位健康年輕人裡陽性類別預期接近 0，並把架構段落的重點放在 class imbalance（weighted loss / focal loss、oversampling）。
- **課綱指定的追問**：「這筆資料能不能訓練出臨床可用的偵測模型？」它的回答：
  > 「簡短回答：**不能。** 只靠這 30 位同學的資料，訓練不出臨床可用的心律不整偵測模型。這不是模型架構或調參的問題，而是資料本身決定了上限。」

  理由有五點：幾乎沒有陽性樣本；30 人族群單一，和需要篩檢的族群不同；標註達不到多位專家仲裁的臨床標準；非醫療級硬體；「臨床可用」在台灣屬醫療器材軟體（SaMD），需要 TFDA 審查與多中心前瞻性驗證。最後建議把題目定位成「健康年輕族群的偽陽性率可行性評估」。
- **前後兩次回答的差異（本則的發現）**：這個模型**沒有**像課程預測的那樣只順著前提回答，第一次就把最根本的問題講在最前面。但第一次回答仍然交出一份「看起來很完整」的計畫，同學資料在其中被降成輔助角色，等於悄悄換掉了原本的研究題目，卻沒有直接說「你原本的題目做不到」。要等到追問，它才明確說「不能」，並補上法規門檻。
- **事實查核**：MIT-BIH 取樣率 360 Hz、inter-patient split（de Chazal 2004）這兩點和我知道的一致。Polar H10 的取樣率、TFDA SaMD 的分類細節，我沒有另外查證（**not verified**）。

**What I changed**
我以後的規則：AI 交出一份完整計畫時，我先自己回答三個問題：陽性樣本從哪來、誰有資格標記、要過哪一關審查。答不出來，就不照計畫做；然後一定追問一句「這筆資料能不能支持這個目的？」，因為「完整的計畫」不等於「題目做得到」。

---

### 2026-11-12 · can my windows support my claim?

**Tool** 同上：Claude Opus 5.5，由 Claude Code 開啟一個**全新、沒有本次對話脈絡**的 Claude 實例來問。我**刻意沒有**告訴它資料只有一個人、一張桌子、一個下午錄的，要看它會不會自己發現。

**What I asked**
> （貼上）硬體與設定（ESP32-S3 + MPU6050、100 Hz、2 s window、8 個特徵、四類）、session A 的訓練集特徵表（min/median/max）、session B 上規則與 MLP 的兩個混淆矩陣、板上即時 20 個 window 的結果（規則 15/20、MLP 20/20）、MLP 29 µs / 704 bytes。
> 「請問根據這些結果，這個模型可以宣稱能做到什麼、不能宣稱什麼？」
>
> 追問（Principle 07）：「我的資料收集方式中，有什麼前提會讓這個模型無法推廣？」

**What it answered**
- **可以宣稱的**：在同一人、同一板、同一安裝方式下，對沒參與訓練的 session 全對，但要附上信賴區間。32/32 的 95% 下界約 89%，B 加即時的 52/52 約 93%，單一類別 8/8 只有約 63%。另外，MLP 在這份資料上贏過這組規則（McNemar p≈0.03，即時 p≈0.06），而且部署成本很小。
- **不能宣稱的**：對其他人、其他安裝方式、其他時間有效；「需要用 ML」是普遍結論，因為它認為我的規則基準偏弱，`az_std` 就能完全分開 shake 和 tap；能在連續真實情境下運作，因為沒有「其他」類別，window 也和動作對齊；把 29 µs 當成系統延遲，實際延遲主要是收滿 2 秒 window。
- **有沒有主動發現限制**（範本列的五項）：
  - 一個受試者：✅ 第一次回答就說「看起來是同一個人」
  - 一張桌子 / 同一安裝：✅ 主動指出 `az_std` 和安裝方向有關
  - 一個下午：✅ 第一次說「其他時間」，追問時明確點出「同一天、同一個地點、前後錄的」
  - 自己起止的 2 秒 window：✅ 主動指出「知道 window 什麼時候開始，然後做一個動作」，真實資料流會有動作轉換、tap 落在邊緣
  - 模擬手勢而非真實事件：🔶 部分。追問時說動作是「刻意、誇張、乾淨」的，但沒有說到「這些不是真實事件」。不過我也沒告訴它這是跌倒偵測的替代練習。
- **追問的回答**：分七類列出隱藏前提，包括晶片特有的校正誤差（still 的 `amag_mean` = 0.955 而不是 1.000）、陀螺儀零偏約 3 dps、100 Hz 對 tap 峰值的取樣運氣、安裝方向、單一使用者的習慣、tilt 速度是隱藏前提、封閉且平衡的四類、樣本獨立性、MLP 外插不可預測。每一項都附了檢驗方法。

**How I checked**
用我的 Record Sheet §5（同一批測試 window 上的規則 vs 模型）、§7（即時 20 個 window），以及 `w10_out/` 的特徵檔核對：
- ✅ **McNemar 和信賴區間**：驗算正確。B 上規則錯 6、MLP 錯 0 → 雙尾精確 p = 2×0.5⁶ ≈ 0.031；即時 5 比 0 → p ≈ 0.063；Clopper-Pearson 下界 0.025^(1/32) ≈ 0.891、0.025^(1/52) ≈ 0.931、0.025^(1/8) ≈ 0.631。
- ✅ **「`az_std` 完全分開 shake 和 tap」**：三份資料都成立。訓練集 A：shake 0.499–0.821 / tap 0.113–0.187；測試集 B：0.552–1.221 / 0.102–0.148；即時：0.677–0.771 / 0.113–0.145。`amag_mean` 也分得開。
- ⚠️ **「大約一半的 tap 低於 2.0 門檻」**：說法誇大。訓練集裡 8 個 tap 只有 3 個 ≤ 2.0（37.5%）。shake「一半 > 2.0」正確（4/8）。
- ⚠️ **「我猜規則在 A 上準確率不會太高」**：實際是 78.1%（`baseline.py` TRAINING 區塊）。
- ❌ **追問中的錯誤前提 1**：它假設板子是「z 軸朝上固定在桌面」。實際上我的 GY-521 是立著插在麵包板上，靜止時重力在 **x 軸**（約 −0.95 g），z 軸是水平的。所以它對 `az_std`「綁死在 z 軸方向」的推論方向對，但對 z 軸代表什麼的理解是錯的。
- ❌ **追問中的錯誤前提 2**：它猜「每類 8 個 window 可能是連續錄的」。實際上 `sketch_a_collect.cpp` 用固定的打散順序（`ORDER`），同一類不會連續出現。
- **它提出、但我的數字不支持（或還沒驗證）的主張**：「經訓練資料調整門檻的簡單規則可能達到相近表現」——我沒有實際做這個規則或淺層決策樹，**not verified**。「模型分 tap/shake 很可能是靠 `az_std`」——我沒有檢查 MLP 的權重或做特徵消融，**not verified**。

**What I changed**
- Record Sheet §10 的「honest limit」（tap 是模擬動作、不是真實跌倒）是在問這個 AI **之前**就寫好的，由 Claude Code 起草、我確認，見下一則。AI 沒有提到「不是真實事件」這點，是因為我沒告訴它用途，所以這個限制不是它幫我找到的。
- 它幫我找到一個我原本沒寫的限制：**最能分開 tap 和 shake 的特徵（`az_std`）是單一軸、和安裝方向有關**。我在 Record Sheet §8 加了一行說明。
- 以後報告準確率時，我會附上樣本數和信賴區間下界，不只寫「100%」。

---

### 2026-11-12 · W10 實驗全程協助（Claude Code，`claude-opus-5-5`）

**What it did**
- 讀完 W10 pack；建立 PlatformIO 專案 `C:\dev\bme3301\w10`。五個 datasheet 常數中，前四個從我 W05 的 `src/main.cpp` 抄過來；第五個 `G_SENS_MDPS = 15.2672` 是它依 register map 的 FS_SEL=1 → 65.5 LSB/°/s 換算的。
- 替我 build、燒錄、開 serial monitor；依我的要求把 Sketch A 的倒數改成 5 秒；處理開 monitor 時自動重置的問題（`--dtr 0 --rts 0`）；刪除兩份中斷的試錄 log。
- 執行 `windows.py`、`baseline.py`、`train.py`、`compare.py`。門檻 **2.0 / 0.15 / 30 是我決定的**，但這組數字和它先前舉的例子相同。
- 把所有數字抄進 Record Sheet，並**起草了所有判斷題的文字**（§1、§2、§3、§5、§6、§7、§8、§9、§10，以及 §0 的兩行規格）。§6 的 RAM 差 52 bytes，是它用 `xtensa-esp32s3-elf-nm` 比對兩個 ELF 找出來的。
- 開兩個全新的 Claude 實例取得上面兩則的回答，並撰寫本檔案。
- 用我提供的專案 API key，以 Edge Impulse ingestion API 上傳 64 個檔案（Training 32、Testing 32，每檔指定標籤）。之後用我建立的臨時 Admin key，透過 Studio API 建立 impulse（2000 ms / 100 Hz / Spectral Analysis / Classification）、產生特徵、以預設值訓練（30 epochs、lr 0.0005）、執行 Classify all，再把混淆矩陣填進 Record Sheet §4、§5。
- 打包要交換給同學的 session A（`swap_for_neighbour.zip`）；收到同學的 session 後跑 neighbour-split 測試（`w10_out_neighbour/`），並把結果與分析寫進 Record Sheet §5、§8、§9、§10。

**How I checked**
- 每個步驟的板上輸出都有 log 可查（`w10/sessionA.txt`、`sessionB.txt`、`live.txt`），build 的 RAM 和講義一致到 byte。
- `G_SENS_MDPS` 的 65.5 LSB/°/s：它說當下沒辦法打開 PDF 確認，請我自己核對 register map。 <!-- 核對後請填上頁碼 -->
- Record Sheet 的判斷題我逐節讀過。§7 原本寫的是它推測的「第一嫌疑」，我把第 44 張投影片的內容貼給它之後才改正。

**What I changed**
- §0 的兩行規格是收集資料**之後**才補寫的，我要求照實註明，不假裝是事先寫好的。
- 判斷題的論點與結論，我確認後保留。
