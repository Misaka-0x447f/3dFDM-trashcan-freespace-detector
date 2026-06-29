// 3D printer waste-bin free-space detector
//
// Pipeline:
//   HC-SR04 中位数采样 → LevelMapper(EMA + 死区) → LedBar / OledView 渲染
//
// 显示语义：剩余空间百分比。料面离探头远 → 桶空 → 100%。

#include <Arduino.h>

#include "config.h"
#include "pins.h"

#include "Ultrasonic.h"
#include "LevelMapper.h"
#include "LedBar.h"
#include "OledView.h"

namespace {
Ultrasonic  us(PIN_US_TRIG, PIN_US_ECHO, cfg::US_TIMEOUT_US);
LevelMapper mapper(cfg::CAL_PCT, cfg::CAL_DIST, cfg::CAL_COUNT, cfg::DISPLAY_DEAD_BAND_PCT);
LedBar      leds(PIN_LED_DIN, cfg::LED_COUNT);
OledView    oled(PIN_I2C_SCL, PIN_I2C_SDA);

uint32_t lastSampleMs = 0;
uint32_t lastRenderMs = 0;
float    lastRawCm    = NAN;
}  // namespace

void setup() {
  Serial.begin(115200);
  delay(50);
  Serial.println();
  Serial.println(F("[boot] trashcan free-space detector"));
  Serial.printf("[cfg] %u calibration points:\n", static_cast<unsigned>(cfg::CAL_COUNT));
  for (size_t i = 0; i < cfg::CAL_COUNT; ++i) {
    Serial.printf("       %6.1f%% -> %5.2fcm\n", cfg::CAL_PCT[i], cfg::CAL_DIST[i]);
  }

  us.begin();
  leds.begin(cfg::LED_BRIGHTNESS);
  oled.begin();
}

void loop() {
  const uint32_t now = millis();

  // 采样：完整中位数窗口为一拍。窗口耗时 ≈ window * interval。
  const uint32_t samplePeriod = cfg::US_MEDIAN_WINDOW * cfg::US_PING_INTERVAL_MS;
  if (now - lastSampleMs >= samplePeriod) {
    lastSampleMs = now;
    float d = us.readMedian(cfg::US_MEDIAN_WINDOW, cfg::US_PING_INTERVAL_MS);
    lastRawCm = d;
    mapper.feed(d);

    if (isnan(d)) {
      Serial.println(F("[us] invalid window"));
    } else {
      Serial.printf("[us] %.1fcm  filt=%.1fcm  %d%%\n",
                    d, mapper.filteredDistance(), mapper.percent());
    }
  }

  // 渲染：百分比节流由 LedBar/OledView 内部 last==now 短路掉。
  if (now - lastRenderMs >= cfg::DISPLAY_REFRESH_MS) {
    lastRenderMs = now;
    const int16_t p = mapper.percent();
    leds.show(p);
    oled.render(p, lastRawCm);
  }

  leds.tick();  // 维持 invalid 闪烁等高频动画
  delay(cfg::LOOP_TICK_MS);  // delay() 在 ESP8266 上喂狗 + yield
}
