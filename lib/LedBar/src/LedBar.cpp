#include "LedBar.h"

#include <Arduino.h>  // millis()

namespace {
constexpr uint32_t kBlinkHalfPeriodMs = 500;  // 500ms 亮 + 500ms 灭 = 1s 周期
}

LedBar::LedBar(uint8_t pin, uint8_t count)
    : _strip(count, pin, NEO_GRB + NEO_KHZ800),
      _lastPercent(kInvalid + 1),  // 错开 sentinel，强制首次必刷
      _isInvalid(false),
      _blinkOn(false),
      _lastBlinkMs(0) {}

void LedBar::begin(uint8_t brightness) {
  _strip.begin();
  _strip.setBrightness(brightness);
  _strip.clear();
  _strip.show();
}

uint32_t LedBar::colorFor(int16_t percent) const {
  // 剩余空间 >=40 → 白（降至 70% 避免比红/黄刺眼）；<40 → 黄；<15 → 红
  if (percent >= 40) return _strip.Color(179, 179, 179);
  if (percent >= 15) return _strip.Color(255, 160, 0);
  return                    _strip.Color(255, 0,   0);
}

void LedBar::show(int16_t percent) {
  if (percent == kInvalid) {
    if (!_isInvalid) {
      // 刚进入 invalid：立刻画一帧红，后续节奏由 tick() 接管
      _isInvalid   = true;
      _blinkOn     = true;
      _lastBlinkMs = millis();
      renderInvalid(true);
    }
    return;
  }

  // 离开 invalid 或正常状态变化
  _isInvalid = false;
  if (percent == _lastPercent) return;
  _lastPercent = percent;
  renderNormal(percent);
}

void LedBar::tick() {
  if (!_isInvalid) return;
  const uint32_t now = millis();
  if (now - _lastBlinkMs >= kBlinkHalfPeriodMs) {
    _lastBlinkMs = now;
    _blinkOn = !_blinkOn;
    renderInvalid(_blinkOn);
  }
}

void LedBar::renderInvalid(bool on) {
  _strip.clear();
  if (on) {
    for (uint16_t i = 0; i < _strip.numPixels(); ++i) {
      _strip.setPixelColor(i, 255, 0, 0);
    }
  }
  _strip.show();
}

void LedBar::renderNormal(int16_t percent) {
  _strip.clear();

  const uint16_t n = _strip.numPixels();
  // LED 物理只有 0..n 颗可点，超界 clamp 用于填充；颜色按真实 percent 算。
  // 下限 5% 保证总有一颗微亮，避免完全不亮误以为掉电。
  int16_t fillPct = percent;
  if (fillPct < 5)   fillPct = 5;
  if (fillPct > 100) fillPct = 100;
  const float    fill  = static_cast<float>(fillPct) * n / 100.0f;
  const uint16_t whole = static_cast<uint16_t>(fill);
  const float    frac  = fill - whole;
  const uint32_t color = colorFor(percent);

  for (uint16_t i = 0; i < whole && i < n; ++i) {
    _strip.setPixelColor(i, color);
  }

  // 边界一颗按余数调暗，画出更平滑的过渡
  if (whole < n && frac > 0.05f) {
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >>  8) & 0xFF;
    uint8_t b =  color        & 0xFF;
    _strip.setPixelColor(whole,
                         static_cast<uint8_t>(r * frac),
                         static_cast<uint8_t>(g * frac),
                         static_cast<uint8_t>(b * frac));
  }

  _strip.show();
}
