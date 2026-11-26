// sketch_3_sparse.cpp - the access pattern that lets a 25-million-parameter table live in
// flash: per token, read a handful of rows out of a table far bigger than any RAM.
//
// A 1 MB table of 64-byte rows (16384 rows). Per "token": 6 rows, chosen by a scramble,
// 384 bytes touched out of 1,048,576. The figure is microseconds per token, from flash and
// from PSRAM, median of W11_REPEATS passes of 256 tokens each.
//
// Plus the trap, made visible: the SAME lookup against a 16 KB table. 16 KB fits inside the
// 32 KB data cache, so after the first pass every row is a cache hit and the number says
// nothing about flash or PSRAM. It is printed under its own name so nobody mistakes it.
#include "w11_common.h"

#define TABLE_BYTES     (1u << 20)                 // 1 MB
#define ROW_BYTES       64
#define ROW_WORDS       (ROW_BYTES / 4)
#define ROWS            (TABLE_BYTES / ROW_BYTES)  // 16384
#define SMALL_ROWS      256                        // 16 KB: the cache-resident control
#define ROWS_PER_TOKEN  6                          // esp32-ai's README: about 6 rows, ~450 B, per token
#define TOKENS          256
#define FLOOR_NS_TOKEN  (ROWS_PER_TOKEN * ROW_WORDS * 1000.0f / 240.0f)  // 96 loads at one cycle each

__attribute__((noinline)) static uint32_t lookupTokens(const uint32_t *table, uint32_t rows) {
  uint32_t s = 0, mask = rows - 1;
  for (uint32_t t = 0; t < TOKENS; t++) {
    for (uint32_t j = 0; j < ROWS_PER_TOKEN; j++) {
      uint32_t r = w11_scramble(t * ROWS_PER_TOKEN + j, mask);   // which row this token needs
      const uint32_t *row = table + r * ROW_WORDS;
      for (int k = 0; k < ROW_WORDS; k++) s += row[k];           // read the whole row
    }
  }
  return s;
}

static void timeLookup(const char *name, const uint32_t *table, uint32_t rows, W11Tier want) {
  Serial.printf("%s_rows = %u\n", name, (unsigned)rows);
  Serial.printf("%s_table_bytes = %u\n", name, (unsigned)(rows * ROW_BYTES));
  if (!w11_checkTier(name, table, want)) return;
  W11Passes p;
  for (int r = 0; r < W11_REPEATS; r++) {
    uint32_t t0 = w11_cycles();
    uint32_t s  = lookupTokens(table, rows);
    uint32_t t1 = w11_cycles();
    p.cycles[r] = t1 - t0;
    p.sum[r] = s;
  }
  float ns = w11_judge(name, p, TOKENS, FLOOR_NS_TOKEN);
  Serial.printf("%s_us_per_token = %s", name, ns < 0 ? "n/a\n" : "");
  if (ns >= 0) Serial.printf("%.2f\n", ns / 1000.0f);
}

static void fill(uint32_t *buf, uint32_t n) {
  for (uint32_t i = 0; i < n; i++) buf[i] = w11_scramble(i, 0xFFFFFFFFu);
}

void setup() {
  Serial.begin(115200);
  delay(3000);
  w11_reportBegin("3_sparse");
  Serial.printf("rows_per_token       = %d\n", ROWS_PER_TOKEN);
  Serial.printf("row_bytes            = %d\n", ROW_BYTES);
  Serial.printf("bytes_per_token      = %d\n", ROWS_PER_TOKEN * ROW_BYTES);
  Serial.printf("tokens_per_pass      = %d\n", TOKENS);

  // Could the full table live in SRAM at all? The heap answers, not a slide.
  size_t largest = heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  Serial.printf("sram_holds_table     = %s (largest internal block %u, table %u)\n",
                largest >= TABLE_BYTES ? "yes" : "no", (unsigned)largest, (unsigned)TABLE_BYTES);

  // ---- flash: 1 MB mapped, then the 16 KB cache-resident control on the same window -----
  const uint32_t *fl = (const uint32_t *)w11_mapFlash(TABLE_BYTES);
  if (!fl) {
    w11_refuseTest("flash1m_lookup",  "could not map 1 MB of the app1 partition");
    w11_refuseTest("flash16k_lookup", "could not map 1 MB of the app1 partition");
  } else {
    timeLookup("flash1m_lookup",  fl, ROWS,       TIER_FLASH);
    timeLookup("flash16k_lookup", fl, SMALL_ROWS, TIER_FLASH);
  }

  // ---- PSRAM: same table size, same rows, same control ------------------------------
  if (w11_psramBytes() == 0) {
    const char *why = "PSRAM heap is 0 bytes: platformio.ini needs board_build.arduino.memory_type = qio_opi and build_flags = -DBOARD_HAS_PSRAM";
    w11_refuseTest("psram1m_lookup",  why);
    w11_refuseTest("psram16k_lookup", why);
  } else {
    uint32_t *ps = (uint32_t *)heap_caps_malloc(TABLE_BYTES, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!ps) {
      w11_refuseTest("psram1m_lookup",  "1 MB PSRAM allocation failed");
      w11_refuseTest("psram16k_lookup", "1 MB PSRAM allocation failed");
    } else {
      fill(ps, TABLE_BYTES / 4);
      timeLookup("psram1m_lookup",  ps, ROWS,       TIER_PSRAM);
      timeLookup("psram16k_lookup", ps, SMALL_ROWS, TIER_PSRAM);
      free(ps);
    }
  }

  // ---- SRAM: the 16 KB control only - the 1 MB table does not fit, by the heap's own count -
  uint32_t *sm = (uint32_t *)heap_caps_malloc(SMALL_ROWS * ROW_BYTES, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  if (!sm) w11_refuseTest("sram16k_lookup", "no internal block of 16 KB free");
  else { fill(sm, SMALL_ROWS * ROW_WORDS); timeLookup("sram16k_lookup", sm, SMALL_ROWS, TIER_SRAM); free(sm); }

  w11_reportEnd();
}

void loop() {}
