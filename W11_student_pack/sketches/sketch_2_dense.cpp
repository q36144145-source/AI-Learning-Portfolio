// sketch_2_dense.cpp - one dense layer, y = W x, 256 inputs to 128 outputs: 32768
// multiply-accumulates per inference. The SAME weights are run six ways:
//     float32 and int8   x   weights in SRAM, in PSRAM, in flash.
// Three things come out, and only one of them is computed rather than measured:
//     size     - bytes of weights          computed from the shape, printed for the record
//     latency  - microseconds per inference, the median of W11_REPEATS passes   MEASURED
//     error    - int8 output against float32 output, on the same input          MEASURED
// The six runs must agree with each other: every float32 run must produce bit-identical
// outputs (same arithmetic, same bytes), and so must every int8 run. That agreement is
// printed, and a block where it fails is refused by report.py.
#include "w11_common.h"
#include <string.h>

#define FLOOR_NS_MAC (1000.0f / 240.0f)   // one cycle per MAC: a scalar kernel cannot beat it

// ---- the two kernels --------------------------------------------------------------
__attribute__((noinline)) static void denseF32(const float *W, const float *x, float *y) {
  for (int o = 0; o < W11_N_OUT; o++) {
    const float *row = W + o * W11_N_IN;
    float acc = 0.0f;
    for (int i = 0; i < W11_N_IN; i++) acc += row[i] * x[i];
    y[o] = acc;
  }
}

// int8 weights, int8 input, int32 accumulator: the shape of esp_nn_fully_connected_s8_ansi
// (reading/esp_nn_fully_connected_ansi.c) with the offsets at zero. Quantisation is a
// MAPPING, not a rounding: w ~= q * SCALE_W, x ~= q * SCALE_X, so y ~= acc * SCALE_W * SCALE_X.
__attribute__((noinline)) static void denseS8(const int8_t *W, const int8_t *x, int32_t *y) {
  for (int o = 0; o < W11_N_OUT; o++) {
    const int8_t *row = W + o * W11_N_IN;
    int32_t acc = 0;
    for (int i = 0; i < W11_N_IN; i++) acc += (int32_t)row[i] * (int32_t)x[i];
    y[o] = acc;
  }
}

// ---- checksums over outputs: the compiler cannot delete work whose result is hashed -----
static uint32_t hashF32(const float *y, int n) {
  uint32_t h = 0;
  for (int i = 0; i < n; i++) { uint32_t b; memcpy(&b, &y[i], 4); h = h * 31u + b; }
  return h;
}
static uint32_t hashS32(const int32_t *y, int n) {
  uint32_t h = 0;
  for (int i = 0; i < n; i++) h = h * 31u + (uint32_t)y[i];
  return h;
}

static float   xF[W11_N_IN];     // inputs always in SRAM, identical for every run
static int8_t  xQ[W11_N_IN];
static float   yF[W11_N_OUT];    // outputs always in SRAM
static int32_t yQ[W11_N_OUT];

static uint32_t f32Hash[3], s8Hash[3];   // one per tier, to prove the six runs agree
static bool     f32Have[3], s8Have[3];

static void runF32(const char *name, const float *W, W11Tier want, int slot) {
  if (!w11_checkTier(name, W, want)) return;
  W11Passes p;
  for (int r = 0; r < W11_REPEATS; r++) {
    uint32_t t0 = w11_cycles();
    denseF32(W, xF, yF);
    uint32_t t1 = w11_cycles();
    p.cycles[r] = t1 - t0;
    p.sum[r] = hashF32(yF, W11_N_OUT);
  }
  float ns = w11_judge(name, p, W11_N_W, FLOOR_NS_MAC);
  Serial.printf("%s_us_per_inference = %s", name, ns < 0 ? "n/a\n" : "");
  if (ns >= 0) Serial.printf("%.1f\n", ns * W11_N_W / 1000.0f);
  f32Hash[slot] = p.sum[0]; f32Have[slot] = true;
}

static void runS8(const char *name, const int8_t *W, W11Tier want, int slot) {
  if (!w11_checkTier(name, W, want)) return;
  W11Passes p;
  for (int r = 0; r < W11_REPEATS; r++) {
    uint32_t t0 = w11_cycles();
    denseS8(W, xQ, yQ);
    uint32_t t1 = w11_cycles();
    p.cycles[r] = t1 - t0;
    p.sum[r] = hashS32(yQ, W11_N_OUT);
  }
  float ns = w11_judge(name, p, W11_N_W, FLOOR_NS_MAC);
  Serial.printf("%s_us_per_inference = %s", name, ns < 0 ? "n/a\n" : "");
  if (ns >= 0) Serial.printf("%.1f\n", ns * W11_N_W / 1000.0f);
  s8Hash[slot] = p.sum[0]; s8Have[slot] = true;
}

static bool allAgree(const uint32_t *h, const bool *have) {
  uint32_t ref = 0; bool got = false;
  for (int i = 0; i < 3; i++) {
    if (!have[i]) continue;
    if (!got) { ref = h[i]; got = true; }
    else if (h[i] != ref) return false;
  }
  return got;
}

void setup() {
  Serial.begin(115200);
  delay(3000);
  w11_reportBegin("2_dense");
  Serial.printf("n_in                 = %d\n", W11_N_IN);
  Serial.printf("n_out                = %d\n", W11_N_OUT);
  Serial.printf("macs_per_inference   = %d\n", W11_N_W);
  Serial.printf("weights_bytes_f32    = %u\n", (unsigned)(W11_N_W * 4));   // computed, not measured
  Serial.printf("weights_bytes_s8     = %u\n", (unsigned)(W11_N_W * 1));   // computed, not measured
  Serial.printf("scale_bytes_s8       = 4\n");                               // one float per tensor
  Serial.printf("scale_w              = %.9e\n", W11_SCALE_W);
  Serial.printf("scale_x              = %.9e\n", W11_SCALE_X);

  // ---- the flash table must be the one the generator wrote ------------------------
  uint32_t sumF = 0, sumQ = 0;
  for (int i = 0; i < W11_N_W; i++) { sumF += W11_W_F32_BITS[i]; sumQ += (uint32_t)(int32_t)W11_W_S8[i]; }
  bool tableOk = (sumF == W11_W_F32_SUM) && (sumQ == W11_W_S8_SUM);
  Serial.printf("table_check          = %s\n", tableOk ? "ok" : "MISMATCH");
  if (!tableOk) {
    Serial.printf("table_check_detail   = f32 0x%08X vs 0x%08X, s8 0x%08X vs 0x%08X\n",
                  (unsigned)sumF, (unsigned)W11_W_F32_SUM, (unsigned)sumQ, (unsigned)W11_W_S8_SUM);
    const char *why = "w11_tables.cpp in src/ is not the one scripts/gen_tables.py writes";
    const char *names[6] = {"f32_sram", "f32_psram", "f32_flash", "s8_sram", "s8_psram", "s8_flash"};
    for (int i = 0; i < 6; i++) w11_refuseTest(names[i], why);
    w11_reportEnd();
    return;
  }

  memcpy(xF, W11_X_F32_BITS, sizeof xF);   // same bit patterns, now typed as float
  memcpy(xQ, W11_X_S8, sizeof xQ);

  const float *wFlashF = (const float *)W11_W_F32_BITS;
  const int8_t *wFlashQ = W11_W_S8;

  // ---- flash: the tables where the linker put them -----------------------------------
  runF32("f32_flash", wFlashF, TIER_FLASH, 2);
  runS8 ("s8_flash",  wFlashQ, TIER_FLASH, 2);

  // ---- SRAM: copied out of flash, one tensor at a time (160 KB would not always fit) ---
  float *wS = (float *)heap_caps_malloc(W11_N_W * 4, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  if (!wS) w11_refuseTest("f32_sram", "no internal block of 128 KB free");
  else { memcpy(wS, wFlashF, W11_N_W * 4); runF32("f32_sram", wS, TIER_SRAM, 0); free(wS); }
  int8_t *wSQ = (int8_t *)heap_caps_malloc(W11_N_W, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  if (!wSQ) w11_refuseTest("s8_sram", "no internal block of 32 KB free");
  else { memcpy(wSQ, wFlashQ, W11_N_W); runS8("s8_sram", wSQ, TIER_SRAM, 0); free(wSQ); }

  // ---- PSRAM: refused outright when the heap has none ---------------------------
  if (w11_psramBytes() == 0) {
    const char *why = "PSRAM heap is 0 bytes: platformio.ini needs board_build.arduino.memory_type = qio_opi and build_flags = -DBOARD_HAS_PSRAM";
    w11_refuseTest("f32_psram", why);
    w11_refuseTest("s8_psram", why);
  } else {
    float *wP = (float *)heap_caps_malloc(W11_N_W * 4, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!wP) w11_refuseTest("f32_psram", "128 KB PSRAM allocation failed");
    else { memcpy(wP, wFlashF, W11_N_W * 4); runF32("f32_psram", wP, TIER_PSRAM, 1); free(wP); }
    int8_t *wPQ = (int8_t *)heap_caps_malloc(W11_N_W, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!wPQ) w11_refuseTest("s8_psram", "32 KB PSRAM allocation failed");
    else { memcpy(wPQ, wFlashQ, W11_N_W); runS8("s8_psram", wPQ, TIER_PSRAM, 1); free(wPQ); }
  }

  // ---- the six runs must agree with each other ----------------------------------------
  Serial.printf("f32_outputs_identical = %s\n", allAgree(f32Hash, f32Have) ? "ok" : "MISMATCH");
  Serial.printf("s8_outputs_identical  = %s\n", allAgree(s8Hash, s8Have)  ? "ok" : "MISMATCH");

  // ---- the quantisation error, measured on this input ----------------------------------
  // yF and yQ still hold the LAST run of each kernel; the runs are identical, so any pair will do.
  denseF32(wFlashF, xF, yF);
  denseS8(wFlashQ, xQ, yQ);
  float deq = W11_SCALE_W * W11_SCALE_X;
  double se = 0, sr = 0, maxAbs = 0;
  for (int o = 0; o < W11_N_OUT; o++) {
    double e = (double)yQ[o] * deq - (double)yF[o];
    se += e * e; sr += (double)yF[o] * (double)yF[o];
    if (fabs(e) > maxAbs) maxAbs = fabs(e);
  }
  double rmsE = sqrt(se / W11_N_OUT), rmsR = sqrt(sr / W11_N_OUT);
  Serial.printf("quant_rms_err         = %.6f\n", rmsE);
  Serial.printf("quant_rms_ref         = %.6f\n", rmsR);
  Serial.printf("quant_rel_rms_err_pct = %s", rmsR > 0 ? "" : "n/a (reference output is zero)\n");
  if (rmsR > 0) Serial.printf("%.4f\n", 100.0 * rmsE / rmsR);
  Serial.printf("quant_max_abs_err     = %.6f\n", maxAbs);
  Serial.printf("quant_half_step_w     = %.6e\n", W11_SCALE_W / 2.0f);   // the per-weight bound: |w - q*s| <= s/2

  w11_reportEnd();
}

void loop() {}
