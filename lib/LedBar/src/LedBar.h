#pragma once

#include <Adafruit_NeoPixel.h>
#include <stdint.h>

class LedBar {
public:
  static constexpr int16_t kInvalid = INT16_MIN;

  LedBar(uint8_t pin, uint8_t count);

  void begin(uint8_t brightness);

  // 显示剩余空间百分比；可为负或超过 100，物理填充按 [5,100] clamp（<5 视为 5，
  // 保证总有一颗微亮）。传 kInvalid 进入红色闪烁告警态，由 tick() 维持节奏。
  void show(int16_t percent);

  // 高频调度（每 loop tick 调用一次），驱动 invalid 状态下的红色闪烁。
  // 非 invalid 态下 no-op，不会偷帧。
  void tick();

private:
  Adafruit_NeoPixel _strip;
  int16_t           _lastPercent;
  bool              _isInvalid;
  bool              _blinkOn;
  uint32_t          _lastBlinkMs;

  uint32_t colorFor(int16_t percent) const;
  void     renderInvalid(bool on);
  void     renderNormal(int16_t percent);
};
