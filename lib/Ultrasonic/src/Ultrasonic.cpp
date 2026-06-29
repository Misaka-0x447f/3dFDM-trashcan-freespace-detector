#include "Ultrasonic.h"

#include <math.h>
#include <stdlib.h>

namespace {
constexpr size_t kMaxWindow = 16;

int cmpFloat(const void* a, const void* b) {
  float fa = *static_cast<const float*>(a);
  float fb = *static_cast<const float*>(b);
  if (fa < fb) return -1;
  if (fa > fb) return 1;
  return 0;
}
}  // namespace

Ultrasonic::Ultrasonic(uint8_t trigPin, uint8_t echoPin, uint32_t timeoutUs)
    : _trig(trigPin), _echo(echoPin), _timeoutUs(timeoutUs) {}

void Ultrasonic::begin() {
  pinMode(_trig, OUTPUT);
  pinMode(_echo, INPUT);
  digitalWrite(_trig, LOW);
}

float Ultrasonic::readOnce() {
  // 触发 10us 高脉冲（前后留低电平整形）
  digitalWrite(_trig, LOW);
  delayMicroseconds(3);
  digitalWrite(_trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(_trig, LOW);

  uint32_t dur = pulseIn(_echo, HIGH, _timeoutUs);
  if (dur == 0) return NAN;

  float cm = dur / 58.0f;
  // 物理无效：盲区内 / 量程外
  if (cm < 2.0f || cm > 400.0f) return NAN;
  return cm;
}

float Ultrasonic::readMedian(size_t window, uint32_t minIntervalMs) {
  if (window == 0) return NAN;
  if (window > kMaxWindow) window = kMaxWindow;

  float buf[kMaxWindow];
  size_t n = 0;

  for (size_t i = 0; i < window; ++i) {
    float v = readOnce();
    if (!isnan(v)) buf[n++] = v;

    if (i + 1 < window) {
      // 用 delay(1) 循环以喂狗 + 让 ESP 跑系统任务
      uint32_t t0 = millis();
      while (millis() - t0 < minIntervalMs) delay(1);
    }
  }

  if (n == 0) return NAN;
  qsort(buf, n, sizeof(float), cmpFloat);
  return buf[n / 2];
}
