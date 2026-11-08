// HX711 原始计数 → 100% + 参考点双点标定 → 剩余空间百分比 → 灯带 / OLED。

#include <Arduino.h>
#include <limits.h>

#include "config.h"
#include "pins.h"
#include "Hx711.h"
#include "WeightMapper.h"
#include "LedBar.h"
#include "OledView.h"

namespace {
Hx711 hx(PIN_HX711_DT, PIN_HX711_SCK);
WeightMapper mapper(cfg::DISPLAY_DEAD_BAND_PCT);
LedBar leds(PIN_LED_DIN, cfg::LED_COUNT);
OledView oled(PIN_I2C_SCL, PIN_I2C_SDA);

int32_t samples[cfg::HX711_MEDIAN_WINDOW] = {};
uint8_t sampleCount = 0;
uint8_t sampleNext = 0;
int32_t lastMedian = INT32_MIN;
uint32_t lastSampleMs = 0;
uint32_t lastLogMs = 0;
uint32_t lastRenderMs = 0;
bool sensorOnline = false;

int32_t medianSample() {
  int32_t sorted[cfg::HX711_MEDIAN_WINDOW];
  for (uint8_t i = 0; i < cfg::HX711_MEDIAN_WINDOW; ++i) sorted[i] = samples[i];
  for (uint8_t i = 1; i < cfg::HX711_MEDIAN_WINDOW; ++i) {
    const int32_t value = sorted[i];
    uint8_t j = i;
    while (j > 0 && sorted[j - 1] > value) {
      sorted[j] = sorted[j - 1];
      --j;
    }
    sorted[j] = value;
  }
  return sorted[cfg::HX711_MEDIAN_WINDOW / 2];
}

void pollScale(uint32_t now) {
  if (hx.ready()) {
    const int32_t raw = hx.read();
    lastSampleMs = now;
    sensorOnline = true;
    samples[sampleNext] = raw;
    sampleNext = (sampleNext + 1) % cfg::HX711_MEDIAN_WINDOW;
    if (sampleCount < cfg::HX711_MEDIAN_WINDOW) ++sampleCount;
    if (sampleCount == cfg::HX711_MEDIAN_WINDOW) {
      lastMedian = medianSample();
      mapper.feed(lastMedian);
      if (now - lastLogMs >= 1000) {
        lastLogMs = now;
        if (mapper.calibrated()) {
          Serial.printf("[hx711] raw=%ld median=%ld filtered=%.0f free=%d%%\n",
                        static_cast<long>(raw), static_cast<long>(lastMedian),
                        mapper.filteredRaw(), mapper.percent());
        } else {
          Serial.printf("[hx711] raw=%ld median=%ld uncalibrated\n",
                        static_cast<long>(raw), static_cast<long>(lastMedian));
        }
      }
    }
  } else if (sensorOnline && now - lastSampleMs > cfg::HX711_STALE_MS) {
    sensorOnline = false;
    sampleCount = 0;
    lastMedian = INT32_MIN;
    mapper.invalidate();
    Serial.println(F("[hx711] timeout; check DT/SCK and power"));
  }
}
}  // namespace

void setup() {
  Serial.begin(115200);
  delay(50);
  Serial.println();
  Serial.println(F("[boot] trashcan free-space detector / HX711"));
  mapper.setCalibration(cfg::CAL_RAW_100, cfg::CAL_RAW_REF, cfg::CAL_PCT_REF);
  if (mapper.calibrated()) {
    Serial.printf("[cal] 100%%=%ld, %.0f%%=%ld\n",
                  static_cast<long>(cfg::CAL_RAW_100), cfg::CAL_PCT_REF,
                  static_cast<long>(cfg::CAL_RAW_REF));
  } else {
    Serial.println(F("[cal] set CAL_RAW_100 and CAL_RAW_REF in include/config.h"));
  }
  hx.begin();
  leds.begin(cfg::LED_BRIGHTNESS);
  oled.begin();
}

void loop() {
  const uint32_t now = millis();
  pollScale(now);

  if (now - lastRenderMs >= cfg::DISPLAY_REFRESH_MS) {
    lastRenderMs = now;
    leds.show(mapper.percent());
    oled.render(mapper.percent(), lastMedian);
  }
  leds.tick();
  delay(cfg::LOOP_TICK_MS);
}
