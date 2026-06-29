#pragma once

#include <Arduino.h>

class Ultrasonic {
public:
  Ultrasonic(uint8_t trigPin, uint8_t echoPin, uint32_t timeoutUs);

  void  begin();

  // 单次触发并读取距离 (cm)；超时/无效返回 NAN。
  float readOnce();

  // 取 window 次中位数，剔除 NAN 后再排序取中。
  // 每次采样之间至少间隔 minIntervalMs（防回波污染）。
  // 全部无效返回 NAN。
  float readMedian(size_t window, uint32_t minIntervalMs);

private:
  uint8_t  _trig;
  uint8_t  _echo;
  uint32_t _timeoutUs;
};
