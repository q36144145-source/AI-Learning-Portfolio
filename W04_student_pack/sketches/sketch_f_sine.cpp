// W04 Sketch F - a known sine, made and measured by the same board.
//
// The ESP32-S3 has no DAC. It does have a sigma-delta modulator, which puts out
// a one-bit stream whose average tracks the duty you write. An RC low-pass turns
// that average into a voltage. You build the RC on your breadboard:
//
//     GPIO2 ---[ R ]---+--- GPIO1 (A0)
//                      |
//                    [ C ]
//                      |
//                     GND
//
// One hardware timer drives BOTH ends: on every tick it writes the next point of
// the sine table AND takes one ADC reading. Generator and sampler therefore share
// one clock, which is what makes the capture COHERENT - an exact whole number of
// sine cycles fits in the record, so the FFT lands in one bin with no leakage and
// SINAD means what the textbook says it means.
//
//   1000 ticks per second / 64 points per cycle = 15.625 Hz
//   2048 samples / 64 points = exactly 32 cycles
//
// WHAT THIS ACTUALLY MEASURES. The tone is made by the modulator, smoothed by YOUR
// R and C, and only then read by the ADC. Everything the FFT counts as noise and
// distortion therefore includes the generator's own quantisation and whatever your
// filter contributes - not the converter's error alone. So the SINAD and the ENOB
// that come out belong to the WHOLE CHAIN, not to the converter. That is the honest
// question anyway: what can this signal chain resolve?
//
// Nothing in this file changes between your two runs. What changes is the R and
// the C on the breadboard. Press the RST button to run it again.
#include <Arduino.h>

#define SDM_PIN    2           // sigma-delta output. GPIO2 sits next to GPIO1 on the header.
#define ADC_PIN    A0          // A0 is GPIO1, an ADC1 channel
#define SDM_CH     0           // sigma-delta channel 0 of 8
#define SDM_FREQ   1220        // the BOTTOM of the range the core accepts (1220 .. 312500).
                               // At 312500 the core sets the prescaler to 0 and GPIO2 toggles at
                               // up to 80 MHz right next to GPIO1; those edges couple into the ADC.
                               // 1220 gives prescaler 255: a 312.5 kHz bit clock, 256x fewer edges.
#define ATTEN      ADC_11db    // keep the same setting you characterised in cycle 1
#define NSINE      64          // points per sine cycle
#define SINE_MID   100.0f      // sine centre, in sigma-delta duty (0 .. 255)
#define SINE_AMP   85.0f       // sine swing: 15 .. 185 of 256 = about 0.19 .. 2.38 V at the pin
#define NCAP       2048        // samples captured = 32 whole cycles
#define PERIOD_US  1000        // 1000 Hz tick

hw_timer_t        *timer = NULL;
SemaphoreHandle_t  due   = NULL;
volatile uint32_t  missed = 0;

uint8_t  table[NSINE];
uint16_t code[NCAP];
uint32_t stamp[NCAP];

void IRAM_ATTR onTick() {
  BaseType_t woken = pdFALSE;
  if (uxSemaphoreGetCount(due)) missed++;
  xSemaphoreGiveFromISR(due, &woken);
  if (woken) portYIELD_FROM_ISR();
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  // 15 .. 185 rather than 0 .. 255: ADC_11db only sees up to about 3.1 V and bends
  // near the top of that, so a sine swinging the full 0 .. 3.3 V would clip and
  // distort before the converter's own limits showed. This one stays inside.
  for (int i = 0; i < NSINE; i++) {
    table[i] = (uint8_t)lroundf(SINE_MID + SINE_AMP * sinf(2.0f * PI * i / NSINE));
  }

  uint32_t carrier = sigmaDeltaSetup(SDM_PIN, SDM_CH, SDM_FREQ);
  sigmaDeltaWrite(SDM_CH, 128);

  analogReadResolution(12);
  analogSetPinAttenuation(ADC_PIN, ATTEN);
  for (int i = 0; i < 16; i++) analogRead(ADC_PIN);

  Serial.println("# W04 sketch F - sigma-delta sine through your RC, sampled by the same clock");
  // sigmaDeltaSetup() measures nothing. It returns the APB clock divided by
  // ((prescaler + 1) x 256) - the carrier its own integer divide implies. The edges
  // your RC actually has to smooth come 256x faster than that.
  Serial.printf ("# carrier the divider implies : %lu Hz (asked for %d - computed, not measured)\n",
                 (unsigned long)carrier, SDM_FREQ);
  Serial.printf ("# the one-bit stream is 256x that : about %lu Hz - that is the edge rate\n",
                 (unsigned long)carrier * 256UL);
  Serial.printf ("# sample rate %d Hz   sine %d points -> %.4f Hz   %d samples = %d cycles\n",
                 1000000 / PERIOD_US, NSINE,
                 (1000000.0 / PERIOD_US) / NSINE, NCAP, NCAP / NSINE);
  Serial.printf ("# attenuation setting %d, pin A0 (GPIO1), sigma-delta out GPIO%d\n",
                 (int)ATTEN, SDM_PIN);
  Serial.printf ("# sine duty %.0f +/- %.0f of 256\n", SINE_MID, SINE_AMP);
  Serial.println("# WRITE DOWN WHICH R AND WHICH C ARE ON THE BREADBOARD RIGHT NOW.");
  Serial.println("# The file cannot tell you, and the two runs look identical without it.");

  // Let the RC settle before the record starts, otherwise the first cycles carry
  // the step response of your own filter and the FFT reports it as distortion.
  due   = xSemaphoreCreateBinary();
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTick, true);
  timerAlarmWrite(timer, PERIOD_US, true);
  timerAlarmEnable(timer);

  uint16_t phase = 0;
  for (int n = 0; n < NSINE * 8; n++) {                 // 8 cycles thrown away
    xSemaphoreTake(due, portMAX_DELAY);
    sigmaDeltaWrite(SDM_CH, table[phase]);
    phase = (phase + 1) % NSINE;
    analogRead(ADC_PIN);
  }
  missed = 0;                                           // the discard phase does not count

  Serial.println("# ---- raw codes follow, one per line ----");
  for (int n = 0; n < NCAP; n++) {
    xSemaphoreTake(due, portMAX_DELAY);
    sigmaDeltaWrite(SDM_CH, table[phase]);
    phase = (phase + 1) % NSINE;
    stamp[n] = micros();
    code[n]  = analogRead(ADC_PIN);
  }
  timerAlarmDisable(timer);
  sigmaDeltaWrite(SDM_CH, 128);

  for (int n = 0; n < NCAP; n++) Serial.println(code[n]);

  uint32_t worst = 0;
  for (int n = 1; n < NCAP; n++) {
    uint32_t span = stamp[n] - stamp[n - 1];
    uint32_t err  = (span > PERIOD_US) ? span - PERIOD_US : PERIOD_US - span;
    if (err > worst) worst = err;
  }
  // Coherence is a claim about the clock. Here is the evidence for it, on your board.
  Serial.printf("# worst sampling interval error: %lu us out of %d us\n", worst, PERIOD_US);
  Serial.printf("# ticks that arrived before the previous read finished: %lu\n", missed);
  Serial.println("# done");
}

void loop() {}
