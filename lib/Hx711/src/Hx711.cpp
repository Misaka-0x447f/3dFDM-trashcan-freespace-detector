#include "Hx711.h"

#include <Arduino.h>

Hx711::Hx711(uint8_t dtPin, uint8_t sckPin) : _dtPin(dtPin), _sckPin(sckPin) {}

void Hx711::begin() {
  pinMode(_dtPin, INPUT);
  digitalWrite(_sckPin, LOW);
  pinMode(_sckPin, OUTPUT);
}

bool Hx711::ready() const {
  return digitalRead(_dtPin) == LOW;
}

int32_t Hx711::read() {
  uint32_t value = 0;
  // HX711 的 SCK 高电平超过 60us 会进入掉电态；短暂屏蔽中断保证时序。
  noInterrupts();
  for (uint8_t i = 0; i < 24; ++i) {
    digitalWrite(_sckPin, HIGH);
    delayMicroseconds(1);
    value = (value << 1) | static_cast<uint32_t>(digitalRead(_dtPin));
    digitalWrite(_sckPin, LOW);
    delayMicroseconds(1);
  }
  digitalWrite(_sckPin, HIGH);  // 第 25 个脉冲：下一次转换仍为 A 通道、增益 128
  delayMicroseconds(1);
  digitalWrite(_sckPin, LOW);
  interrupts();

  if (value & 0x800000UL) value |= 0xFF000000UL;
  return static_cast<int32_t>(value);
}
