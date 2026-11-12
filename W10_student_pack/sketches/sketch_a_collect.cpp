// Sketch A - collect one session of labelled windows. One run = one session = one log.
//
// The board prompts, you move, the board records. Thirty-two windows: eight of each of
// the four classes, interleaved in a fixed order so that "late in the session" is not
// quietly the same thing as "one particular class". Each window is 2.0 s at 100 Hz,
// paced by a hardware timer, buffered in RAM, and printed afterwards - the print never
// sets the pace (W06's logger did that on purpose; this one does not).
//
// Every window ends with its own account: ticks, missed, clipped, read_failed. A
// window with a missed tick has a broken time axis; a window with a clipped sample
// has a peak that is not the peak. windows.py drops both, with the count, rather than
// compute a feature from them.
//
// Run it twice. Session A is what you train on; session B, recorded after you have put
// the board down and picked it up again, is what you test on. Never split one session's
// windows at random - windows.py refuses to, and the slides say why.
#include "w10_imu.h"

#define REPS 8                             // windows per class per session
// A fixed interleaving. Every student records the classes in this same order, so the
// TAs can predict what window 17 should be, and no class is always "the last one".
static const uint8_t ORDER[REPS][N_CLASSES] = {
  {0,1,2,3}, {2,0,3,1}, {1,3,0,2}, {3,2,1,0}, {0,2,1,3}, {1,0,3,2}, {3,1,2,0}, {2,3,0,1} };

static const char *HOWTO[N_CLASSES] = {
  "put the board flat on the desk and take your hand off it",
  "hold it level; on GO, roll it slowly through 90 degrees and back",
  "hold it; on GO, shake it steadily, about three times a second",
  "hold it level; on GO, tap it firmly on the desk edge once or twice" };

Window win;

static void dumpWindow(int n, int cls, const Window *w) {
  Serial.printf("W %d %s begin\n", n, CLASS_NAMES[cls]);
  for (int i = 0; i < WIN_SAMPLES; i++)
    Serial.printf("%d,%.3f,%.3f,%.3f,%.1f,%.1f,%.1f\n", i,
                  w->ax[i], w->ay[i], w->az[i], w->gx[i], w->gy[i], w->gz[i]);
  Serial.printf("W %d %s end ticks=%u missed=%u clipped=%u read_failed=%u\n",
                n, CLASS_NAMES[cls], w->ticks, w->missed, w->clipped, w->read_failed);
}

void setup() {
  Serial.begin(115200);
  delay(3000);
  if (!imuBringUp()) return;                 // a halt line was printed; nothing below runs
  printHeader("collect - one session");
  Serial.printf("# reps_per_class=%d windows=%d\n", REPS, REPS * N_CLASSES);
  Serial.println("# columns: i,ax_g,ay_g,az_g,gx_dps,gy_dps,gz_dps");
  Serial.println("# Watch the screen. Each window: a prompt, 3-2-1, GO, two seconds, STOP.");
  timerStart();

  uint32_t perClass[N_CLASSES] = {0, 0, 0, 0};
  int n = 0;
  for (int rep = 0; rep < REPS; rep++) {
    for (int k = 0; k < N_CLASSES; k++) {
      int cls = ORDER[rep][k]; n++;
      Serial.printf("\n>> window %d of %d : %s - %s\n", n, REPS * N_CLASSES, CLASS_NAMES[cls], HOWTO[cls]);
      for (int c = 3; c > 0; c--) { Serial.printf(">> %d\n", c); delay(1000); }
      Serial.println(">> GO");
      bool ok = captureWindow(&win);          // 2.0 s, paced by the timer
      Serial.println(">> STOP - hold on, printing");
      if (!ok) { Serial.println("halt=bus_died_in_window"); return; }
      dumpWindow(n, cls, &win);
      perClass[cls]++;
    }
  }
  Serial.printf("# done windows=%d still=%u tilt=%u shake=%u tap=%u\n", n,
                perClass[0], perClass[1], perClass[2], perClass[3]);
  Serial.println("# One run = one session. Save this log, then run it again for the other session.");
}

void loop() {}
