// W04 Sketch E - the converter on its own.
// Same 250 Hz schedule you verified in W03. Last week the thing being measured
// was time. This week it is a voltage, and the clock is the instrument.
//
// ONE RUN = ONE CONDITION. You change the two lines below (ATTEN, BIAS) and reflash.
// Four runs today. Nothing else in this file ever changes.
#include <Arduino.h>
#include "driver/adc.h"
#include "driver/rtc_io.h"
#include "esp_adc_cal.h"

#define ADC_PIN    A0          // A0 is GPIO1, an ADC1 channel. Right-hand header, next to GPIO2.
#define ATTEN      ADC_11db    // <-- run 3 changes this to ADC_0db. Choices: ADC_0db ADC_2_5db ADC_6db ADC_11db
#define BIAS       0           // <-- 1 for runs 2 and 3 (nothing on A0), 0 for runs 1 and 4 (a wire on A0).
                               // 1 switches on the pin's own pull-up AND pull-down resistors, which
                               // hold A0 part-way up the supply: a voltage that is on neither rail.
#define N          2000        // 2000 samples at 250 Hz = 8 s
#define PERIOD_US  4000        // 250 Hz. The interval W03 told you to trust.

hw_timer_t        *timer = NULL;
SemaphoreHandle_t  due   = NULL;
volatile uint32_t  missed = 0;        // ticks that arrived before the previous read finished

uint16_t code[N];
uint32_t stamp[N];

// analogRead() resets the pin on every call, and that switches the pull resistors off.
// So after setup this sketch reads the ADC directly, and converts to millivolts with
// the same factory calibration analogReadMilliVolts() uses.
adc1_channel_t                ch;
esp_adc_cal_characteristics_t cal;
uint16_t readCode()               { return adc1_get_raw(ch); }
uint32_t toMilliVolts(uint16_t c) { return esp_adc_cal_raw_to_voltage(c, &cal); }

// The ISR does NOT read the ADC. The ADC driver lives in flash and takes locks;
// calling it from an interrupt handler is how a working program starts failing
// at random. The ISR hands the job to the main task and returns immediately.
// You will meet this pattern again, with a name, in W07.
void IRAM_ATTR onTick() {
  BaseType_t woken = pdFALSE;
  if (uxSemaphoreGetCount(due)) missed++;      // previous tick not collected yet
  xSemaphoreGiveFromISR(due, &woken);
  if (woken) portYIELD_FROM_ISR();
}

void setup() {
  Serial.begin(115200);
  delay(2000);                                 // let the monitor attach

  analogReadResolution(12);
  analogSetPinAttenuation(ADC_PIN, ATTEN);

  analogRead(ADC_PIN);                                      // lets the core hand the pin to the ADC

  ch = (adc1_channel_t)digitalPinToAnalogChannel(ADC_PIN);
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ch, (adc_atten_t)ATTEN);
  esp_adc_cal_characterize(ADC_UNIT_1, (adc_atten_t)ATTEN, ADC_WIDTH_BIT_12, 1100, &cal);
  if (BIAS) {
    rtc_gpio_pullup_en((gpio_num_t)ADC_PIN);
    rtc_gpio_pulldown_en((gpio_num_t)ADC_PIN);
  } else {
    rtc_gpio_pullup_dis((gpio_num_t)ADC_PIN);
    rtc_gpio_pulldown_dis((gpio_num_t)ADC_PIN);
  }
  delay(100);                                               // let the pin settle

  for (int i = 0; i < 16; i++) readCode();                  // warm-up, discarded

  uint32_t sum_code = 0, sum_mv = 0;
  for (int i = 0; i < 16; i++) {
    sum_code += readCode();
    sum_mv   += toMilliVolts(readCode());                   // the chip's factory calibration
  }
  float avg_code = sum_code / 16.0f;
  float avg_mv   = sum_mv   / 16.0f;

  Serial.println("# W04 sketch E - the converter on its own");
  Serial.printf ("# pin A0 (GPIO1), attenuation setting %d, %d samples every %d us\n",
                 (int)ATTEN, N, PERIOD_US);
  Serial.printf ("# input bias : %s\n", BIAS ? "internal pull-up + pull-down ON (nothing on A0)"
                                             : "OFF (A0 is whatever is wired to it)");
  // Those two averages came from SEPARATE reads, and the millivolt one is the chip's
  // own factory calibration applied to its own raw read. Their ratio is that curve's
  // slope at ONE point - a scale factor, not a calibration, and no proof of linearity.
  Serial.printf ("# mV and raw code  : %.1f mV, code %.1f (two separate reads)\n", avg_mv, avg_code);
  if (avg_code > 20.0f && avg_code < 4000.0f) {
    Serial.printf("# scale factor     : %.4f mV per code, at that one point\n",
                  avg_mv / avg_code);
    Serial.printf("# the same x 4095  : %.0f mV - extrapolation, not a measured full scale\n",
                  avg_mv * 4095.0f / avg_code);
  } else {
    Serial.println("# scale factor     : NOT COMPUTABLE - the input is sitting on a rail.");
    Serial.println("#                    That is a result. Write it down and say which rail.");
  }
  Serial.println("# ---- raw codes follow, one per line ----");

  due   = xSemaphoreCreateBinary();
  timer = timerBegin(0, 80, true);             // 80 MHz / 80 = 1 MHz, so 1 tick = 1 us
  timerAttachInterrupt(timer, &onTick, true);
  timerAlarmWrite(timer, PERIOD_US, true);
  timerAlarmEnable(timer);

  for (int n = 0; n < N; n++) {
    xSemaphoreTake(due, portMAX_DELAY);        // blocks: no busy-wait, no watchdog
    stamp[n] = micros();
    code[n]  = readCode();
  }
  timerAlarmDisable(timer);

  for (int n = 0; n < N; n++) Serial.println(code[n]);

  uint32_t worst = 0;
  for (int n = 1; n < N; n++) {
    uint32_t span = stamp[n] - stamp[n - 1];
    uint32_t err  = (span > PERIOD_US) ? span - PERIOD_US : PERIOD_US - span;
    if (err > worst) worst = err;
  }
  Serial.printf("# worst sampling interval error: %lu us out of %d us\n", worst, PERIOD_US);
  Serial.printf("# ticks that arrived before the previous read finished: %lu\n", missed);
  Serial.println("# done");
}

void loop() {}
