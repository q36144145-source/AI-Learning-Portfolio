// Sketch C - both classifiers on the board, on the same window, timed the same way.
//
// The RULE is the kind of thing you wrote in W08: three constants, each read off your
// own feature table, applied in a fixed order. It needs no training and no model file.
// The MODEL is the one scripts/train.py wrote into model_weights.h: two small matrices,
// a ReLU, a softmax. Both see the same eight features from the same captured window.
//
// What this sketch refuses to do:
//   - run with a threshold still at its shipped 0.0 (a rule that always fires is not a rule)
//   - call the model while model_weights.h is the placeholder (MODEL_READY 0): it says so
//     on every line instead of printing a class name that came from zeros
//   - time anything with a Serial.print inside the timed region. The three *_us figures
//     are taken with micros() around the computation only; printing happens afterwards.
//
// It stops after LIVE_WINDOWS windows so that one run is one log, as in every week.
#include "w10_imu.h"
#include "model_weights.h"

#define LIVE_WINDOWS 20                    // 5 per class is the record sheet's protocol

// ---- the rule: three blanks, from YOUR feature table (windows.py, feature_table.txt) ----
// Applied in this order. Zero means unfilled, and the sketch halts on it.
volatile const float T_TAP_G     = 2.0f;    // amag_max above this     -> tap
volatile const float T_SHAKE_G   = 0.15f;   // else amag_std above this -> shake
volatile const float T_TILT_DPS  = 30.0f;   // else gmag_mean above this -> tilt
                                           // else                      -> still
static int ruleInfer(const float *f) {
  if (f[2] > T_TAP_G)    return 3;
  if (f[1] > T_SHAKE_G)  return 2;
  if (f[4] > T_TILT_DPS) return 1;
  return 0;
}

// ---- the model: standardise, one hidden layer, softmax. Weights from model_weights.h ----
static int mlpInfer(const float *f, float *pmax) {
  float x[N_FEAT], h[N_HID], z[N_CLS];
  for (int i = 0; i < N_FEAT; i++) x[i] = (f[i] - MODEL_MU[i]) / MODEL_SD[i];
  for (int j = 0; j < N_HID; j++) {
    float s = MODEL_B1[j];
    for (int i = 0; i < N_FEAT; i++) s += MODEL_W1[j][i] * x[i];
    h[j] = s > 0 ? s : 0;                               // ReLU
  }
  float zmax = -1e30f;
  for (int c = 0; c < N_CLS; c++) {
    float s = MODEL_B2[c];
    for (int j = 0; j < N_HID; j++) s += MODEL_W2[c][j] * h[j];
    z[c] = s; if (s > zmax) zmax = s;
  }
  float den = 0; for (int c = 0; c < N_CLS; c++) den += expf(z[c] - zmax);
  int best = 0; *pmax = 0;
  for (int c = 0; c < N_CLS; c++) { float p = expf(z[c] - zmax) / den; if (p > *pmax) { *pmax = p; best = c; } }
  return best;
}

static const size_t MODEL_BYTES = sizeof(MODEL_MU) + sizeof(MODEL_SD) + sizeof(MODEL_W1)
                                + sizeof(MODEL_B1) + sizeof(MODEL_W2) + sizeof(MODEL_B2);
// Read through `volatile` so the compiler keeps the model path (and its weights in Flash)
// even while the header is the placeholder: RAM is then identical before and after training
// and Flash moves only by the provenance string - which is what the record sheet says.
volatile const int modelReady = MODEL_READY;
Window win;
float feat[N_FEAT];

void setup() {
  Serial.begin(115200);
  delay(3000);
  if (!imuBringUp()) return;
  if (T_TAP_G <= 0 || T_SHAKE_G <= 0 || T_TILT_DPS <= 0) {
    Serial.println("halt=threshold_not_filled");
    Serial.println("# T_TAP_G, T_SHAKE_G, T_TILT_DPS come from YOUR feature_table.txt. Fill all three.");
    return;
  }
  printHeader("classify - rule and model, live");
  Serial.printf("# model_ready=%d model_bytes=%u n_feat=%d n_hid=%d n_cls=%d\n",
                (int)modelReady, (unsigned)MODEL_BYTES, N_FEAT, N_HID, N_CLS);
  Serial.printf("# model_provenance=%s\n", MODEL_PROVENANCE);
  Serial.printf("# rule T_TAP_G=%.3f T_SHAKE_G=%.3f T_TILT_DPS=%.1f\n", (float)T_TAP_G, (float)T_SHAKE_G, (float)T_TILT_DPS);
  if (!modelReady) Serial.println("# model=NOT READY: model_weights.h is the placeholder. The rule still runs; the model column says none.");
  Serial.println("# columns: L,n,rule,model,p_model,feat_us,rule_us,mlp_us,missed,clipped,f0..f7");
  timerStart();

  for (int n = 1; n <= LIVE_WINDOWS; n++) {
    Serial.printf("\n>> live window %d of %d : do ONE class, and write down which\n", n, LIVE_WINDOWS);
    for (int c = 3; c > 0; c--) { Serial.printf(">> %d\n", c); delay(1000); }
    Serial.println(">> GO");
    if (!captureWindow(&win)) { Serial.println("halt=bus_died_in_window"); return; }
    Serial.println(">> STOP");

    uint32_t t0 = micros();
    computeFeatures(&win, feat);
    uint32_t t1 = micros();
    int r = ruleInfer(feat);
    uint32_t t2 = micros();
    int m = -1; float p = 0;
    if (modelReady) m = mlpInfer(feat, &p);
    uint32_t t3 = micros();
    // Nothing above this line printed. Everything below is outside the timed regions.
    Serial.printf("L,%d,%s,%s,%.2f,%u,%u,%u,%u,%u", n, CLASS_NAMES[r],
                  m < 0 ? "none" : CLASS_NAMES[m], p,
                  t1 - t0, t2 - t1, modelReady ? (t3 - t2) : 0, win.missed, win.clipped);
    for (int i = 0; i < N_FEAT; i++) Serial.printf(",%.3f", feat[i]);
    Serial.println();
  }
  Serial.printf("# done live_windows=%d\n", LIVE_WINDOWS);
}

void loop() {}
