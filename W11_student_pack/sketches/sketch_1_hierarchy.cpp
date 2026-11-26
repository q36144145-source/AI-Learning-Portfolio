// sketch_1_hierarchy.cpp - the same read pattern against three memories.
//
// Two patterns: SEQUENTIAL (word after word) and RANDOM (every word once, scrambled
// order). Three places: internal SRAM, external PSRAM, memory-mapped flash. The figure
// is nanoseconds per 32-bit word, the median of W11_REPEATS passes. Nothing is printed
// that the sketch cannot stand behind: see w11_judge() in w11_common.h for the rules.
#include "w11_common.h"

#define WORDS_128K   (131072 / 4)      // 32768 words: the working set every tier gets
#define WORDS_1M     (1048576 / 4)     // 262144 words: PSRAM and flash only - SRAM cannot hold it
#define FLOOR_NS     (1000.0f / 240.0f) // one CPU cycle. No load can take less; below it the loop was deleted

// ---- the three kernels. noinline: the timer brackets a real call, not a hoisted loop ----
__attribute__((noinline)) static uint32_t sumSeq(const uint32_t *p, uint32_t n) {
  uint32_t s = 0;
  for (uint32_t i = 0; i < n; i++) s += p[i];
  return s;
}

__attribute__((noinline)) static uint32_t sumRand(const uint32_t *p, uint32_t n) {
  uint32_t s = 0, mask = n - 1;
  for (uint32_t i = 0; i < n; i++) s += p[w11_scramble(i, mask)];
  return s;
}

// The control: the random loop with the memory read taken out. Whatever this costs is
// the loop and the index arithmetic, and it is inside every "rand" figure above.
__attribute__((noinline)) static uint32_t sumIndexOnly(const uint32_t *unused, uint32_t n) {
  (void)unused;
  uint32_t s = 0, mask = n - 1;
  for (uint32_t i = 0; i < n; i++) s += w11_scramble(i, mask);
  return s;
}

typedef uint32_t (*Kernel)(const uint32_t *, uint32_t);

static void timePasses(W11Passes &p, Kernel fn, const uint32_t *buf, uint32_t n) {
  for (int r = 0; r < W11_REPEATS; r++) {
    uint32_t t0 = w11_cycles();
    uint32_t s  = fn(buf, n);
    uint32_t t1 = w11_cycles();
    p.cycles[r] = t1 - t0;
    p.sum[r]    = s;
  }
}

static void fill(uint32_t *buf, uint32_t n) {   // the same bytes in every RAM buffer, on every board
  for (uint32_t i = 0; i < n; i++) buf[i] = w11_scramble(i, 0xFFFFFFFFu);
}

// One tier, one working set: sequential, then random, then the balance between them.
// A full permutation reads every word exactly once, so the two checksums MUST agree.
static void testTier(const char *tag, const uint32_t *buf, uint32_t n, W11Tier want) {
  char seqName[40], randName[40];
  snprintf(seqName,  sizeof seqName,  "%s_seq",  tag);
  snprintf(randName, sizeof randName, "%s_rand", tag);
  Serial.printf("%s_words = %u\n", tag, (unsigned)n);
  if (!w11_checkTier(tag, buf, want)) {
    w11_refuseTest(seqName,  "wrong tier, see above");
    w11_refuseTest(randName, "wrong tier, see above");
    return;
  }
  W11Passes seq, rnd;
  timePasses(seq, sumSeq,  buf, n);
  timePasses(rnd, sumRand, buf, n);
  const char *balance = (seq.sum[0] == rnd.sum[0]) ? NULL
                      : "sequential and random passes did not read the same words";
  Serial.printf("%s_balance = %s\n", tag, balance ? "MISMATCH" : "ok");
  w11_judge(seqName,  seq, n, FLOOR_NS, balance);
  w11_judge(randName, rnd, n, FLOOR_NS, balance);
}

void setup() {
  Serial.begin(115200);
  delay(3000);                      // give the monitor time to attach; the report prints once
  w11_reportBegin("1_hierarchy");
  Serial.printf("words_128k           = %u\n", (unsigned)WORDS_128K);
  Serial.printf("words_1m             = %u\n", (unsigned)WORDS_1M);

  // ---- the hierarchy is on the pin header ---------------------------------------
  w11_pinModeChecked(35, OUTPUT);   // refused: an octal PSRAM line on the N16R8
  w11_pinModeChecked(2, OUTPUT);    // fine
  Serial.printf("pin_2                = driven\n");

  // ---- where does a plain malloc() land, and does anything tell you? ----------------
  void *m2k  = malloc(2048);
  void *m64k = malloc(65536);
  Serial.printf("malloc_2k_tier       = %s\n", m2k  ? w11_tierName(w11_tierOf(m2k))  : "FAILED");
  Serial.printf("malloc_64k_tier      = %s\n", m64k ? w11_tierName(w11_tierOf(m64k)) : "FAILED");
  free(m2k); free(m64k);

  // ---- SRAM: 128 KB if the heap has it, 64 KB if not; the size used is printed -------
  uint32_t nS = WORDS_128K;
  uint32_t *sram = (uint32_t *)heap_caps_malloc(nS * 4, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  if (!sram) { nS = WORDS_128K / 2; sram = (uint32_t *)heap_caps_malloc(nS * 4, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT); }
  if (!sram) {
    w11_refuseTest("sram_seq",  "no internal block of 64 KB free");
    w11_refuseTest("sram_rand", "no internal block of 64 KB free");
  } else {
    fill(sram, nS);
    testTier("sram", sram, nS, TIER_SRAM);
    free(sram);
  }

  // ---- PSRAM: refused outright when the heap has none ---------------------------
  if (w11_psramBytes() == 0) {
    const char *why = "PSRAM heap is 0 bytes: platformio.ini needs board_build.arduino.memory_type = qio_opi and build_flags = -DBOARD_HAS_PSRAM";
    w11_refuseTest("psram128k_seq", why);  w11_refuseTest("psram128k_rand", why);
    w11_refuseTest("psram1m_seq",   why);  w11_refuseTest("psram1m_rand",   why);
  } else {
    uint32_t *ps = (uint32_t *)heap_caps_malloc(WORDS_1M * 4, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!ps) {
      w11_refuseTest("psram128k_seq", "1 MB PSRAM allocation failed"); w11_refuseTest("psram128k_rand", "1 MB PSRAM allocation failed");
      w11_refuseTest("psram1m_seq",   "1 MB PSRAM allocation failed"); w11_refuseTest("psram1m_rand",   "1 MB PSRAM allocation failed");
    } else {
      fill(ps, WORDS_1M);
      testTier("psram128k", ps, WORDS_128K, TIER_PSRAM);   // the first 128 KB of the same buffer
      testTier("psram1m",   ps, WORDS_1M,   TIER_PSRAM);
      free(ps);
    }
  }

  // ---- flash: a 1 MB window of the unused OTA slot, mapped like const data (XIP) --------
  const uint32_t *fl = (const uint32_t *)w11_mapFlash(WORDS_1M * 4);
  if (!fl) {
    w11_refuseTest("flash128k_seq", "could not map 1 MB of the app1 partition"); w11_refuseTest("flash128k_rand", "could not map 1 MB of the app1 partition");
    w11_refuseTest("flash1m_seq",   "could not map 1 MB of the app1 partition"); w11_refuseTest("flash1m_rand",   "could not map 1 MB of the app1 partition");
  } else {
    testTier("flash128k", fl, WORDS_128K, TIER_FLASH);
    testTier("flash1m",   fl, WORDS_1M,   TIER_FLASH);
  }

  // ---- the control: the random loop without the memory ----------------------------
  W11Passes ctl;
  timePasses(ctl, sumIndexOnly, NULL, WORDS_1M);
  w11_judge("control_noload", ctl, WORDS_1M, FLOOR_NS);

  w11_reportEnd();
}

void loop() {}
