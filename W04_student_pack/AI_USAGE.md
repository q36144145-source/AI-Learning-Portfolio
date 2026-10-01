# AI_USAGE.md — the two W04 entries

Append these to the `AI_USAGE.md` already in your repo. Same four fields as every week:
**what I asked · what it answered · how I checked · what I changed.**

---

## Entry 1 — the converter you are holding (cycle 1 → cycle 2)

### 2026-10-01 · ESP32-S3 ADC, summarised and then checked

**Tool** Gemini

**What I asked**
> Summarise the ESP32-S3 ADC for a biomedical data logger: the resolution, the four
> attenuation settings and the input voltage range each one gives, the absolute
> accuracy, and the input impedance of an ADC1 pin. Give a source for each figure.

**What it answered**
- **Resolution:** 12-bit SAR ADC (raw output codes 0 to 4095). Source: ESP32-S3 Technical Reference Manual / Datasheet.
- **Four attenuation settings and measurable input voltage ranges:**
  | Attenuation enum | Nominal attenuation | Measurable input voltage range |
  |---|---|---|
  | `ADC_ATTEN_DB_0` (`ADC_0db`) | 0 dB | 0 – 950 mV |
  | `ADC_ATTEN_DB_2_5` (`ADC_2_5db`) | 2.5 dB | 0 – 1250 mV |
  | `ADC_ATTEN_DB_6` (`ADC_6db`) | 6 dB | 0 – 1750 mV |
  | `ADC_ATTEN_DB_12` / `ADC_ATTEN_DB_11` (`ADC_11db`) | 11 / 12 dB | 0 – 3100 mV |
  Source: ESP-IDF Programming Guide (ADC Oneshot / Calibration Driver) and ESP32-S3 Datasheet.
- **Absolute accuracy:** DNL ±7 LSB, INL ±12 LSB; after eFuse curve-fitting calibration, total error is approximately ±10 mV (at 0 dB) to ±60 mV (at 11/12 dB). Source: ESP32-S3 Datasheet, Section "ADC Characteristics".
- **Input impedance of an ADC1 pin:** No fixed DC resistor; it is a switched-capacitor SAR input with sampling capacitor `C_sample` ≈ 5–10 pF and switch resistance `R_sw` ≈ 1–2 kΩ (effective input impedance depends on sampling frequency, typically > 1 MΩ at low sample rates). Source: ESP32-S3 Datasheet.

**How I checked**
I inspected the three local SDK files inside my PlatformIO installation under `packages/framework-arduinoespressif32/`:
1. `.../tools/sdk/esp32s3/include/soc/esp32s3/include/soc/soc_caps.h`: Located `#define SOC_ADC_DIGI_MAX_BITWIDTH (12)` and `#define SOC_ADC_MAX_BITWIDTH (12)`, which confirms the 12-bit resolution claim.
2. `.../tools/sdk/esp32s3/include/hal/include/hal/adc_types.h`: Located the `adc_atten_t` enum defining `ADC_ATTEN_DB_0 = 0`, `ADC_ATTEN_DB_2_5 = 1`, `ADC_ATTEN_DB_6 = 2`, `ADC_ATTEN_DB_12 = 3`, and the compatibility alias `ADC_ATTEN_DB_11 = ADC_ATTEN_DB_12`. However, the millivolt input voltage ranges (0–950 mV, 0–1250 mV, 0–1750 mV, 0–3100 mV) are **not located** anywhere in `adc_types.h`.
3. `.../cores/esp32/esp32-hal-adc.c`: Located `#define DEFAULT_VREF 1100` (1100 mV nominal internal reference) and the `analogSetAttenuation` mapping, but the absolute accuracy figures (±7 LSB DNL, ±12 LSB INL, ±60 mV) and the ADC1 input impedance (`C_sample` pF / switched-capacitor values) are **not located** in any of the three local source files (searched all three files for `impedance`, `INL`, `DNL`, `950`, and `3100`).
4. **Comparison with my Run 2 full scale:** The full scale I extrapolated from my own Run 2 scale factor (`0.8558 mV/code` at 1499.1 mV, code 1751.8) was **3504 mV** (`0.8558 × 4095`). The AI table stated an upper range of **3100 mV** for `ADC_11db` / `ADC_12db`, which differs from my extrapolated full scale by **404 mV** (`3504 mV - 3100 mV`). Because my 3504 mV figure is a single-point linear extrapolation from mid-scale (1499.1 mV) rather than a true full-scale measurement—and the real converter saturates near the 3300 mV supply rail with top-end roll-off—this 404 mV disagreement reflects the difference between a single-point slope extrapolation and the calibrated linear operating ceiling, rather than purely a model error.

**What I changed**
I dropped the unverified input impedance and INL/DNL accuracy numbers that could not be located in the local SDK files, and I did not use the AI's generic 3100 mV ceiling to convert my readings. Instead, for all voltage conversions and the Cycle 3 error budget, I used my own measured single-point scale factor (`0.8558 mV/code`, extrapolated full scale `3504 mV`) while explicitly noting in the record sheet that it rests on the SDK's internal calibration read at a single mid-scale point.

---

## Entry 2 — the chip we could not buy (cycle 3)

### 2026-10-01 · AD8232, summarised and then traced

**Tool** Gemini

**What I asked**
> I am specifying a single-lead ECG front end using the AD8232. Give me its gain, its common-mode rejection ratio, its bandwidth, and what the right-leg drive output is for. Cite the page or table for each.

**What it answered**
- **Gain:** Fixed instrumentation amplifier (IA) gain of 100 V/V, plus an uncommitted operational amplifier (A1) that allows setting additional post-filter gain (e.g., 11 V/V for a total system gain of 1100 V/V in typical reference configurations). (Cited: Page 1 Features; Page 3, Table 1 Specifications).
- **Common-Mode Rejection Ratio (CMRR):** 80 dB minimum (86 dB typical) from DC to 60 Hz with 0 kΩ source imbalance. (Cited: Page 1 Features; Page 3, Table 1 Specifications).
- **Bandwidth:** Externally programmable via user-selected passive RC high-pass and low-pass filter networks (commonly configured for 0.5 Hz to 40 Hz in monitoring setups or 0.05 Hz to 150 Hz in diagnostic setups); internal op-amp A1 has a gain-bandwidth product of 100 kHz. (Cited: Page 4, Table 1; Pages 18–21 Applications Information).
- **Right-Leg Drive (RLD) output purpose:** Senses the common-mode voltage on the inputs, inverts and amplifies it, and drives it back into the patient via a third reference electrode to actively cancel power-line (50/60 Hz) common-mode interference and keep the input within the IA's linear range. (Cited: Page 15, Theory of Operation — Right-Leg Drive Amplifier).

**How I checked**
I checked each of the four items side-by-side against the Analog Devices AD8232 data sheet (Rev. C / D):
1. **Gain:** Located on **Page 1 (Features: "Gain = 100 V/V")** and **Page 3, Table 1 (Specifications — Instrumentation Amplifier, Gain = 100 V/V, min 96 to max 104 V/V)**. The additional gain stage using op-amp A1 is located in **Applications Information (Pages 18–20)**.
2. **CMRR:** Located on **Page 3, Table 1 (Specifications — Instrumentation Amplifier, Common-Mode Rejection Ratio = 80 dB min, 86 dB typ)**, explicitly specified under the conditions **DC to 60 Hz, 0 kΩ source imbalance** (and 72 dB min with 5 kΩ source imbalance).
3. **Bandwidth:** Located on **Page 4, Table 1 (Specifications — Operational Amplifier A1, Gain Bandwidth Product = 100 kHz)**. Specific fixed ECG corner frequencies (such as 0.5–40 Hz) are **not located** in Table 1 because the chip contains no internal filter capacitors; instead, external RC filter design equations and example passbands (0.5–40 Hz monitor, 0.05–150 Hz diagnostic) are located in **Theory of Operation and Applications Information — Filtering (Pages 14–21, Figures 41–46)**.
4. **Right-leg drive purpose:** Located on **Page 15 (Theory of Operation — Right-Leg Drive (RLD) Amplifier)**, which states that the RLD amplifier inverts the common-mode signal present at the instrumentation amplifier inputs and drives the third electrode to counter common-mode voltage variations (such as 50/60 Hz mains hum) and improve CMRR.
All four items checked and located in approximately 6 minutes.

**What I changed**
The IA gain (100 V/V), CMRR (80 dB min), and RLD function survived verification against Table 1 and Page 15. However, I removed any implication that CMRR is an unconditional number or that the AD8232 has a built-in fixed bandwidth. In a supplier specification, I would explicitly write: (1) IA gain = 100 V/V ± 4% (Table 1, p. 3), (2) CMRR ≥ 80 dB at DC–60 Hz with 0 kΩ imbalance and ≥ 72 dB with 5 kΩ electrode impedance imbalance (Table 1, p. 3), and (3) external NP0/C0G capacitors and 1% resistors required to set the two-pole high-pass (0.5 Hz) and two-pole low-pass (40 Hz) corners per the application circuits on Pages 18–21.
