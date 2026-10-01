// W04 Sketch G - BONUS. Not run in class. Same rig, no new parts.
//
// Sketch F asks "how good is this converter at one amplitude, at one frequency".
// This one asks a different question: "is the curve between the two ends straight".
// It parks the sigma-delta at 17 fixed duty values, waits for your RC to settle,
// and takes 200 readings at each. linfit.py fits one straight line through all of
// them and tells you the worst distance from it, in codes.
//
// A converter whose transfer curve bends is not fixed by averaging, and it is not
// visible in a standard deviation taken at one point. It is a different defect and
// it needs a different measurement. That is the whole reason this file exists.
//
// Wiring is unchanged from Sketch F. Leave the breadboard alone.
#include <Arduino.h>

#define SDM_PIN    2
#define ADC_PIN    A0
#define SDM_CH     0
#define SDM_FREQ   1220        // the bottom of the range (was 312500). At 312500 the core sets prescaler 0
                               // and GPIO2 toggles at up to 80 MHz next to GPIO1; see Sketch F.
#define ATTEN      ADC_11db
#define NSTEP      17          // duty 0, 16, 32 ... 256 -> clipped to 255
#define NREAD      200         // readings per step
#define SETTLE_MS  120         // must be >> the time constant of YOUR R and C

void setup() {
  Serial.begin(115200);
  delay(2000);

  uint32_t carrier = sigmaDeltaSetup(SDM_PIN, SDM_CH, SDM_FREQ);
  analogReadResolution(12);
  analogSetPinAttenuation(ADC_PIN, ATTEN);

  Serial.println("# W04 sketch G - transfer curve of the whole chain");
  Serial.printf ("# carrier the board actually achieved : %lu Hz\n", (unsigned long)carrier);
  Serial.printf ("# %d steps, %d readings each, %d ms settling, attenuation setting %d\n",
                 NSTEP, NREAD, SETTLE_MS, (int)ATTEN);
  Serial.println("# NOTE the settling time has to be much longer than R times C.");
  Serial.println("#      If it is not, every step carries the tail of the one before it,");
  Serial.println("#      and the curve you measure is your own impatience, not the converter.");
  Serial.println("# ---- duty,code pairs follow ----");

  for (int s = 0; s < NSTEP; s++) {
    int duty = s * 16;
    if (duty > 255) duty = 255;
    sigmaDeltaWrite(SDM_CH, (uint8_t)duty);
    delay(SETTLE_MS);
    for (int i = 0; i < 8; i++) analogRead(ADC_PIN);       // discard the settling reads
    for (int i = 0; i < NREAD; i++) {
      Serial.printf("%d,%d\n", duty, analogRead(ADC_PIN));
    }
  }

  sigmaDeltaWrite(SDM_CH, 128);
  Serial.println("# done");
}

void loop() {}
