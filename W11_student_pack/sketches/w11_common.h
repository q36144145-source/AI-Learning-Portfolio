// w11_common.h - everything that must NOT differ between the three sketches: the clock,
// the test that says which memory a pointer is in, the report format, and the rules by
// which a figure is REFUSED. The only thing under test today is where the bytes live.
#pragma once
#include <Arduino.h>
#include <esp_heap_caps.h>
#include <esp_partition.h>
#include <soc/soc_memory_types.h>   // esp_ptr_internal / esp_ptr_external_ram / esp_ptr_in_drom

// ---- the measurement contract ---------------------------------------------
#define W11_REPEATS         5     // every figure is the MEDIAN of this many passes
#define W11_SPREAD_MAX_PCT  25    // (max - min) / median over passes 2-5 above this -> REFUSED, unstable
#define W11_MIN_CYCLES      20000 // a pass shorter than this is too short to time -> REFUSED
#define W11_DCACHE_BYTES    32768 // sdkconfig: CONFIG_ESP32S3_DATA_CACHE_SIZE
#define W11_LINE_BYTES      32    // sdkconfig: CONFIG_ESP32S3_DATA_CACHE_LINE_SIZE

// ---- the clock -------------------------------------------------------------
// The CPU cycle counter: one count per cycle, 240 per microsecond. micros() ticks once
// per microsecond and the things timed today take 4 to 400 ns EACH, so a single access
// cannot be timed - a pass has to be long enough that the total is far above the
// resolution. W11_MIN_CYCLES is that rule, and a pass that breaks it is refused.
static inline uint32_t w11_cycles() { return ESP.getCycleCount(); }
static inline float w11_nsPerItem(uint32_t cycles, uint32_t items) {
  return 1000.0f * (float)cycles / (float)getCpuFrequencyMhz() / (float)items;
}

// ---- the tiers: which memory is this pointer in? -----------------------------
// ESP32-S3 data addresses (soc.h): internal SRAM 0x3FC88000-0x3FD00000, external
// PSRAM 0x3D000000-0x3E000000, memory-mapped flash 0x3C000000-0x3D000000. The address
// IS the hierarchy. Note the order below: esp_ptr_in_drom() is true for the whole
// 0x3C000000-0x3E000000 window, PSRAM included, so PSRAM is tested first.
enum W11Tier { TIER_SRAM, TIER_PSRAM, TIER_FLASH, TIER_UNKNOWN };

static const char *w11_tierName(W11Tier t) {
  switch (t) {
    case TIER_SRAM:  return "SRAM";
    case TIER_PSRAM: return "PSRAM";
    case TIER_FLASH: return "FLASH";
    default:         return "UNKNOWN";
  }
}

static W11Tier w11_tierOf(const void *p) {
  if (esp_ptr_internal(p))     return TIER_SRAM;
  if (esp_ptr_external_ram(p)) return TIER_PSRAM;
  if (esp_ptr_in_drom(p))      return TIER_FLASH;
  return TIER_UNKNOWN;
}

// PSRAM: psramFound() is NOT the test. In this build's sdkconfig the PSRAM is brought
// up by the bootloader (CONFIG_SPIRAM_BOOT_INIT) and psramInit() then returns true
// without looking, so psramFound() says true even on a build where the init failed and
// the system carried on (CONFIG_SPIRAM_IGNORE_NOTFOUND). The heap knows how many
// bytes it was actually given. Zero means there is no PSRAM to measure.
static inline size_t w11_psramBytes() { return heap_caps_get_total_size(MALLOC_CAP_SPIRAM); }

// GPIO 35, 36 and 37 on the N16R8 module are the octal PSRAM's own lines,
// brought out to the header anyway. pinMode(35, OUTPUT) compiles, and then
// corrupts the memory you are about to benchmark. So this one refuses.
static bool w11_pinModeChecked(int pin, uint8_t mode) {
  if (pin == 35 || pin == 36 || pin == 37) {
    Serial.printf("pin_refused          = GPIO%d: wired to the octal PSRAM "
                  "on this module (psram_bytes=%u), not driven\n",
                  pin, (unsigned)w11_psramBytes());
    return false;
  }
  pinMode(pin, mode);
  return true;
}

// ---- the report ---------------------------------------------------------------
static void w11_reportBegin(const char *sketch) {
  Serial.println();
  Serial.println("=== W11 REPORT ===");
  Serial.printf("sketch               = %s\n", sketch);
  Serial.printf("cpu_mhz              = %u\n", (unsigned)getCpuFrequencyMhz());
  Serial.printf("repeats              = %d\n", W11_REPEATS);
  Serial.printf("spread_max_pct       = %d\n", W11_SPREAD_MAX_PCT);
  Serial.printf("min_cycles           = %d\n", W11_MIN_CYCLES);
  Serial.printf("dcache_bytes         = %d\n", W11_DCACHE_BYTES);
  Serial.printf("line_bytes           = %d\n", W11_LINE_BYTES);
  Serial.printf("psram_found_api      = %d\n", (int)psramFound());        // what the API claims
  Serial.printf("psram_bytes          = %u\n", (unsigned)w11_psramBytes()); // what the heap has
  Serial.printf("internal_largest     = %u\n",
                (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
}

static void w11_reportEnd() { Serial.println("=== END ==="); }

// One timed test = W11_REPEATS passes of the same work. Each pass returns a checksum so
// that the compiler cannot delete the work, and so that two passes that should have read
// the same bytes can be shown to have done so.
typedef struct {
  uint32_t cycles[W11_REPEATS];
  uint32_t sum[W11_REPEATS];
} W11Passes;

static uint32_t w11_median(uint32_t *v, int n) {   // sorts in place; n is small
  for (int i = 1; i < n; i++) { uint32_t k = v[i]; int j = i - 1; while (j >= 0 && v[j] > k) { v[j + 1] = v[j]; j--; } v[j + 1] = k; }
  return v[n / 2];
}

// Judge one test and print its lines. `items` is what a figure is "per" (words, MACs,
// tokens). `floor_ns` is the smallest per-item time the hardware could possibly give:
// below it the compiler removed the work, and the figure is refused rather than printed.
// `extra`, when not NULL, is a reason the caller already has to refuse (for example the
// sequential and random passes of the same buffer disagreeing on their checksum).
// Returns the median ns per item, or a negative number when refused.
static float w11_judge(const char *name, const W11Passes &p, uint32_t items, float floor_ns,
                       const char *extra = NULL) {
  uint32_t c[W11_REPEATS];
  uint32_t cmin = 0xFFFFFFFFu, cmax = 0;
  bool sumsAgree = true;
  for (int i = 0; i < W11_REPEATS; i++) {
    c[i] = p.cycles[i];
    if (i > 0 && c[i] < cmin) cmin = c[i];   // spread over passes 2-5: pass 1 is the cold one
    if (i > 0 && c[i] > cmax) cmax = c[i];   // and is printed on its own as _first_ns_per_item
    if (p.sum[i] != p.sum[0]) sumsAgree = false;
  }
  uint32_t med = w11_median(c, W11_REPEATS);
  float spread = med ? 100.0f * (float)(cmax - cmin) / (float)med : 0.0f;
  float ns = w11_nsPerItem(med, items);
  const char *refuse = extra;
  if (refuse)                    { /* the caller's reason comes first */ }
  else if (!sumsAgree)           refuse = "passes read different data (checksums differ)";
  else if (med < W11_MIN_CYCLES) refuse = "pass too short to time";
  else if (ns < floor_ns)        refuse = "faster than the hardware floor: the compiler removed the work";
  else if (spread > W11_SPREAD_MAX_PCT) refuse = "unstable across passes";

  if (refuse) {
    Serial.printf("%s_status = REFUSED: %s (median_cycles=%u spread_pct=%.1f ns_per_item=%.2f)\n",
                  name, refuse, (unsigned)med, spread, ns);
    Serial.printf("%s_ns_per_item = n/a\n", name);
  } else {
    Serial.printf("%s_status = ok\n", name);
    Serial.printf("%s_ns_per_item = %.2f\n", name, ns);
  }
  Serial.printf("%s_first_ns_per_item = %.2f\n", name, w11_nsPerItem(p.cycles[0], items)); // pass 1, cache state unknown
  Serial.printf("%s_spread_pct = %.1f\n", name, spread);
  Serial.printf("%s_checksum = 0x%08X\n", name, (unsigned)p.sum[0]);
  return refuse ? -1.0f : ns;
}

static void w11_refuseTest(const char *name, const char *why) {
  Serial.printf("%s_status = REFUSED: %s\n", name, why);
  Serial.printf("%s_ns_per_item = n/a\n", name);
}

// A pointer that was supposed to be in one tier and is in another is the most expensive
// mistake available today: the number would look right and mean nothing.
static bool w11_checkTier(const char *name, const void *p, W11Tier want) {
  W11Tier got = w11_tierOf(p);
  Serial.printf("%s_addr = 0x%08X\n", name, (unsigned)(uintptr_t)p);
  Serial.printf("%s_tier = %s\n", name, w11_tierName(got));
  if (got != want) {
    char why[96];
    snprintf(why, sizeof why, "buffer is in %s, not %s", w11_tierName(got), w11_tierName(want));
    w11_refuseTest(name, why);
    return false;
  }
  return true;
}

// Memory-mapped flash: a window into the unused OTA slot (app1, 0x330000 bytes), read
// through the cache exactly the way code and const data are - execute-in-place (XIP).
// This is how esp32-ai reads its 25M-parameter table: from flash, by address.
static const void *w11_mapFlash(size_t bytes) {
  const esp_partition_t *part = esp_partition_find_first(ESP_PARTITION_TYPE_APP,
                                                         ESP_PARTITION_SUBTYPE_APP_OTA_1, NULL);
  if (!part || part->size < bytes) return NULL;
  const void *ptr = NULL;
  spi_flash_mmap_handle_t h;
  if (esp_partition_mmap(part, 0, bytes, SPI_FLASH_MMAP_DATA, &ptr, &h) != ESP_OK) return NULL;
  return ptr;
}

// Bijection on [0, n) for n a power of two: xorshift (invertible) then odd multiply
// (invertible mod 2^k). Every index appears exactly once, in a scrambled order, so a
// "random" pass reads exactly the same words as the sequential pass - which is what lets
// the two checksums be compared.
static inline uint32_t w11_scramble(uint32_t i, uint32_t mask) {
  i ^= i >> 7;  i = (i * 0x9E3779B1u) & mask;
  i ^= i >> 5;  i = (i * 0x85EBCA77u) & mask;
  return i;
}

// ---- the dense layer shared by sketches 2 and 3 --------------------------------
#define W11_N_IN   256
#define W11_N_OUT  128
#define W11_N_W    (W11_N_IN * W11_N_OUT)   // 32768 weights

extern const uint32_t W11_W_F32_SUM;
extern const uint32_t W11_W_S8_SUM;
extern const float    W11_SCALE_W;
extern const float    W11_SCALE_X;
extern const uint32_t W11_W_F32_BITS[W11_N_W];
extern const int8_t   W11_W_S8[W11_N_W];
extern const uint32_t W11_X_F32_BITS[W11_N_IN];
extern const int8_t   W11_X_S8[W11_N_IN];
