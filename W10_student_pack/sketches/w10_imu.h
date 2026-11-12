// w10_imu.h - the IMU you brought up in W05, the window you log in W10, and the
// eight numbers both sketches compute from it. Included by sketch_a_collect.cpp and
// sketch_c_classify.cpp so that the data you train on and the data the board
// classifies are shaped by the SAME code.
//
// Lineage. The bring-up below is W05's sketch_4_init_fixed.cpp: a read returns success
// and value separately, every write is ACK-checked, the part is identified by WHO_AM_I
// before it is configured, and the first frame waits for the device's own data-ready
// bit. The five blanks are the four you filled from the datasheet in W05 plus one more
// (gyro sensitivity). Copy your W05 values in. Until they are filled, both sketches
// refuse to run, exactly as sketch 4 did - a window logged under an unknown scale is
// not a window, it is a column of unlabelled integers.
//
// What is new. A hardware timer paces the reads at FS_HZ (W03's lesson: the loop does
// not set the pace, the timer does), a window is WIN_SAMPLES consecutive frames, and
// every window reports how many ticks it missed and how many samples hit full scale.
// windows.py drops any window where either is non-zero, with the reason, rather than
// hand you a feature computed on a broken time axis or a clipped peak.
#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <math.h>

// ---- the signal contract -------------------------------------------------
#define FS_HZ          100                 // the timer's rate, not the loop's
#define PERIOD_US      (1000000 / FS_HZ)   // 10000 us
#define WIN_SAMPLES    200                 // 2.0 s per window
#define N_CLASSES      4
static const char *CLASS_NAMES[N_CLASSES] = { "still", "tilt", "shake", "tap" };
// Index order is fixed here and in scripts/w10lib.py. Do not reorder one without the other.

// ---- MPU6050 registers (W05) ----------------------------------------------
#define WHO_AM_I      0x75
#define PWR_MGMT_1    0x6B  // this part boots asleep
#define SMPLRT_DIV    0x19  // register 25: sample rate = 1 kHz / (1 + SMPLRT_DIV)
#define CONFIG        0x1A  // register 26: DLPF_CFG, bits 2:0
#define GYRO_CONFIG   0x1B
#define ACCEL_CONFIG  0x1C
#define INT_ENABLE    0x38  // bit 0 DATA_RDY_EN
#define INT_STATUS    0x3A  // bit 0 DATA_RDY_INT, cleared by reading it
#define ACCEL_XOUT_H  0x3B  // 14 bytes from here: accel xyz, temp, gyro xyz, high byte first

// ---- the five blanks. From the datasheet, as in W05. Not from a slide, not from a model.
// `volatile` keeps the compiler from folding them away, so the build size is the same
// whether they are filled or not.
volatile const uint8_t WHOAMI_EXPECT   = 0x00; // 1. identity byte for the part you have
volatile const uint8_t ACCEL_CFG_VALUE = 0x00; // 2. accel full scale, one byte (W05 used +/-4 g)
volatile const uint8_t GYRO_CFG_VALUE  = 0x00; // 3. gyro full scale, one byte (W05 used +/-500 dps)
volatile const float   XL_SENS_MG      = 0.0f; // 4. accel sensitivity at THAT range, mg per LSB
volatile const float   G_SENS_MDPS     = 0.0f; // 5. gyro sensitivity at THAT range, milli-dps per LSB
// -----
// Not blanks: the digital low-pass and the divider that make the part produce new
// samples at FS_HZ with a 44 Hz accelerometer bandwidth (register map, registers 25-26).
// Check them against the datasheet if you like; they are not something you are asked to find.
const uint8_t  DLPF_CFG_VALUE   = 0x03;  // accel BW 44 Hz, gyro BW 42 Hz, internal rate 1 kHz
const uint8_t  SMPLRT_DIV_VALUE = (1000 / FS_HZ) - 1;  // 9 -> 100 Hz
const uint32_t XL_STARTUP_MS     = 30;   // datasheet maximum, as in W05
const uint32_t SETTLE_TIMEOUT_MS = 500;
const int16_t  CLIP_LSB          = 32700; // |raw| at or beyond this counts as full scale

#define IMU_ADDR_A 0x68
#define IMU_ADDR_B 0x69   // AD0 tied high - W05 found some boards there
static uint8_t imu_addr = 0;

// ---- Wire helpers: same contract as W05 sketch 4 ---------------------------
static bool w8(uint8_t reg, uint8_t val) {           // true only if the device acknowledged
  Wire.beginTransmission(imu_addr); Wire.write(reg); Wire.write(val);
  return Wire.endTransmission() == 0;
}
static bool r8(uint8_t reg, uint8_t *val) {          // success and value, separately
  Wire.beginTransmission(imu_addr); Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom((uint8_t)imu_addr, (uint8_t)1) != 1) return false;
  *val = Wire.read();
  return true;
}
// One 14-byte burst: ax ay az temp gx gy gz, each int16 high byte first.
static bool readFrame14(int16_t *a, int16_t *g) {
  Wire.beginTransmission(imu_addr); Wire.write((uint8_t)ACCEL_XOUT_H);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom((uint8_t)imu_addr, (uint8_t)14) != 14) return false;
  int16_t v[7];
  for (int k = 0; k < 7; k++) { uint8_t hi = Wire.read(), lo = Wire.read(); v[k] = (int16_t)((hi << 8) | lo); }
  a[0] = v[0]; a[1] = v[1]; a[2] = v[2];              // v[3] is temperature; not used
  g[0] = v[4]; g[1] = v[5]; g[2] = v[6];
  return true;
}

// The five blanks, checked before the bus is touched. An unfilled blank is not a value.
static const char *unfilledBlank() {
  if (WHOAMI_EXPECT   == 0x00)  return "1 WHOAMI_EXPECT";
  if (ACCEL_CFG_VALUE == 0x00)  return "2 ACCEL_CFG_VALUE";   // legal byte, wrong answer
  if (GYRO_CFG_VALUE  == 0x00)  return "3 GYRO_CFG_VALUE";    // to the question asked
  if (XL_SENS_MG      <= 0.0f)  return "4 XL_SENS_MG";
  if (G_SENS_MDPS     <= 0.0f)  return "5 G_SENS_MDPS";
  return NULL;
}

// Bring the part up in an order you can defend. Prints a `halt=` line and returns false
// at the first thing that is not right; nothing after a halt means anything.
static bool imuBringUp() {
  const char *blank = unfilledBlank();                 // 0. before touching the bus at all
  if (blank) {
    Serial.printf("halt=blank_not_filled blank=%s\n", blank);
    Serial.println("# That constant is still at its shipped zero. Copy your W05 datasheet values in.");
    return false;
  }
  Wire.begin();                                        // SDA GPIO 8, SCL GPIO 9 on this board
  Wire.setClock(400000);
  uint8_t id = 0;
  const uint8_t addrs[2] = { IMU_ADDR_A, IMU_ADDR_B };
  for (int i = 0; i < 2 && imu_addr == 0; i++) {       // 1. who is there, and does it ACK
    imu_addr = addrs[i];
    if (!r8(WHO_AM_I, &id)) imu_addr = 0;
  }
  if (imu_addr == 0) {
    Serial.println("halt=identity_read_failed");
    Serial.println("# Nothing acknowledged at 0x68 or 0x69. VCC 3V3, GND, SDA GPIO 8, SCL GPIO 9 - W05 sketch 1.");
    return false;
  }
  Serial.printf("# who_am_i=0x%02X expected=0x%02X addr=0x%02X\n", id, (uint8_t)WHOAMI_EXPECT, imu_addr);
  if (id != WHOAMI_EXPECT) { Serial.println("halt=identity_mismatch"); return false; }

  struct Step { uint8_t reg, val; const char *what; };
  Step init[] = {                                      // 2. configure, checking every ACK
    { PWR_MGMT_1,   0x00,                      "PWR_MGMT_1 wake" },
    { SMPLRT_DIV,   SMPLRT_DIV_VALUE,          "SMPLRT_DIV" },
    { CONFIG,       DLPF_CFG_VALUE,            "CONFIG DLPF" },
    { ACCEL_CONFIG, (uint8_t)ACCEL_CFG_VALUE,  "ACCEL_CONFIG" },
    { GYRO_CONFIG,  (uint8_t)GYRO_CFG_VALUE,   "GYRO_CONFIG" },
    { INT_ENABLE,   0x01,                      "INT_ENABLE DATA_RDY_EN" },
  };
  for (int i = 0; i < 6; i++) {
    if (!w8(init[i].reg, init[i].val)) {
      Serial.printf("halt=write_nacked step=%d reg=%s\n", i + 1, init[i].what);
      return false;                                    // one silent NACK and every number
    }                                                  // after it would be invented
  }
  delay(XL_STARTUP_MS);                                // 3. the datasheet's start-up time
  uint32_t guard = millis();                           // 4. then the device's own say-so
  for (;;) {
    uint8_t st = 0;
    if (!r8(INT_STATUS, &st)) { Serial.println("halt=bus_stopped_answering"); return false; }
    if (st & 0x01) break;
    if (millis() - guard > SETTLE_TIMEOUT_MS) { Serial.println("halt=no_data_ready_before_timeout"); return false; }
  }
  return true;
}

// ---- the timer: W03's metronome. The ISR only counts; the loop does the I2C. ----
static hw_timer_t *w10timer = NULL;
static volatile uint32_t tickCount = 0;   // every alarm
static volatile bool     tickFresh = false;
static volatile uint32_t tickMissed = 0;  // an alarm arrived while the previous one was still unread

static void IRAM_ATTR onW10Tick() {
  tickCount++;
  if (tickFresh) tickMissed++;            // the loop had not consumed the last tick: a hole
  tickFresh = true;
}
static void timerStart() {
  w10timer = timerBegin(0, 80, true);     // 1 MHz base
  timerAttachInterrupt(w10timer, &onW10Tick, true);
  timerAlarmWrite(w10timer, PERIOD_US, true);
  timerAlarmEnable(w10timer);
}

// ---- one window: WIN_SAMPLES frames at FS_HZ, in physical units ----------------
typedef struct {
  float ax[WIN_SAMPLES], ay[WIN_SAMPLES], az[WIN_SAMPLES];   // g
  float gx[WIN_SAMPLES], gy[WIN_SAMPLES], gz[WIN_SAMPLES];   // dps
  uint32_t ticks, missed, clipped, read_failed;
} Window;

// Fill one window. Blocks for WIN_SAMPLES ticks. Returns false if the bus died.
static bool captureWindow(Window *w) {
  w->ticks = 0; w->missed = 0; w->clipped = 0; w->read_failed = 0;
  const float ka = XL_SENS_MG / 1000.0f, kg = G_SENS_MDPS / 1000.0f;
  noInterrupts(); tickFresh = false; tickMissed = 0; interrupts();
  for (int i = 0; i < WIN_SAMPLES; ) {
    if (!tickFresh) continue;                          // wait for the metronome
    tickFresh = false;
    int16_t a[3], g[3];
    if (!readFrame14(a, g)) { w->read_failed++; if (w->read_failed > 5) return false; continue; }
    w->ticks++;
    for (int k = 0; k < 3; k++) if (abs(a[k]) >= CLIP_LSB || abs(g[k]) >= CLIP_LSB) { w->clipped++; break; }
    w->ax[i] = a[0] * ka; w->ay[i] = a[1] * ka; w->az[i] = a[2] * ka;
    w->gx[i] = g[0] * kg; w->gy[i] = g[1] * kg; w->gz[i] = g[2] * kg;
    i++;
  }
  w->missed = tickMissed;
  return true;
}

// ---- the eight features. scripts/w10lib.py computes the same eight, in this order. ----
#define N_FEAT 8
static const char *FEAT_NAMES[N_FEAT] = {
  "amag_mean", "amag_std", "amag_max", "amag_min", "gmag_mean", "gmag_max", "az_std", "amag_zc" };

static void computeFeatures(const Window *w, float *f) {
  float amag[WIN_SAMPLES];
  float asum = 0, amax = -1e9f, amin = 1e9f, gsum = 0, gmax = -1e9f, azsum = 0;
  for (int i = 0; i < WIN_SAMPLES; i++) {
    amag[i] = sqrtf(w->ax[i]*w->ax[i] + w->ay[i]*w->ay[i] + w->az[i]*w->az[i]);
    float gm = sqrtf(w->gx[i]*w->gx[i] + w->gy[i]*w->gy[i] + w->gz[i]*w->gz[i]);
    asum += amag[i]; if (amag[i] > amax) amax = amag[i]; if (amag[i] < amin) amin = amag[i];
    gsum += gm; if (gm > gmax) gmax = gm;
    azsum += w->az[i];
  }
  float amean = asum / WIN_SAMPLES, azmean = azsum / WIN_SAMPLES;
  float avar = 0, azvar = 0; int zc = 0;
  for (int i = 0; i < WIN_SAMPLES; i++) {
    float d = amag[i] - amean; avar += d * d;
    float e = w->az[i] - azmean; azvar += e * e;
    if (i > 0 && ((amag[i-1] - amean) < 0) != (d < 0)) zc++;
  }
  f[0] = amean;
  f[1] = sqrtf(avar / WIN_SAMPLES);     // population std, as numpy's default
  f[2] = amax;
  f[3] = amin;
  f[4] = gsum / WIN_SAMPLES;
  f[5] = gmax;
  f[6] = sqrtf(azvar / WIN_SAMPLES);
  f[7] = (float)zc;
}

static void printHeader(const char *sketch) {
  Serial.printf("# W10 %s\n", sketch);
  Serial.printf("# source=mpu6050 addr=0x%02X\n", imu_addr);
  Serial.printf("# fs_hz=%d window_samples=%d n_feat=%d\n", FS_HZ, WIN_SAMPLES, N_FEAT);
  Serial.printf("# classes=%s,%s,%s,%s\n", CLASS_NAMES[0], CLASS_NAMES[1], CLASS_NAMES[2], CLASS_NAMES[3]);
  Serial.printf("# accel_cfg=0x%02X xl_sens_mg=%.4f gyro_cfg=0x%02X g_sens_mdps=%.4f\n",
                (uint8_t)ACCEL_CFG_VALUE, (float)XL_SENS_MG, (uint8_t)GYRO_CFG_VALUE, (float)G_SENS_MDPS);
}
